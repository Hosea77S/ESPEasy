# include "../PluginStructs/P153_data_struct.h"

# include <ESPeasySerial.h>

P153_data_struct::~P153_data_struct() 
{
    if (easySerial != nullptr) 
    {
        delete easySerial;
        easySerial = nullptr;
    }
}

void P153_data_struct::reset() 
{
    if (easySerial != nullptr) {
        delete easySerial;
        easySerial = nullptr;
    }
}

bool P153_data_struct::init(ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx, unsigned long baudrate,
                            uint8_t config) 
{
    if ((serial_rx < 0) && (serial_tx < 0)) 
    {
        return false;
    }
    reset();
    easySerial = new (std::nothrow) ESPeasySerial(port, serial_rx, serial_tx);

    if (isInitialized()) 
    {
        # if defined(ESP8266)
        easySerial->begin(baudrate, (SerialConfig)config);
        # elif defined(ESP32)
        easySerial->begin(baudrate, config);
        # endif // if defined(ESP8266)
        return true;
    }
    return false;
}

bool P153_data_struct::isInitialized() const 
{
    return easySerial != nullptr;
}

void P153_data_struct::sendString(const String& data) 
{
    if (isInitialized() && (!data.isEmpty())) 
    {
        // setDisableFilterWindowTimer();
        easySerial->write(data.c_str());

        if (loglevelActiveFor(LOG_LEVEL_INFO)) 
        {
            String log = F("Proxy: Sending: ");
            log += data;
            addLogMove(LOG_LEVEL_INFO, log);
        }
    }
}

void P153_data_struct::sendData(uint8_t *data, size_t size) 
{
    if (isInitialized() && size) 
    {
        //setDisableFilterWindowTimer();
        easySerial->write(data, size);

        if (loglevelActiveFor(LOG_LEVEL_INFO)) 
        {
            String log = F("Proxy: Sending ");
            log += size;
            log += F(" bytes.");
            addLogMove(LOG_LEVEL_INFO, log);
        }
    }
}

bool P153_data_struct::loop() 
{
    if (!isInitialized()) 
    {
        return false;
    }
    bool fullDataReceived = false;

    if (easySerial != nullptr) 
    {
        int available = easySerial->available();

        while (available > 0 && !fullDataReceived) 
        {
            char c = easySerial->read();
            --available;

            if (available == 0) 
            {
                available = easySerial->available();
                delay(0);
            }

            switch (currentState) 
            {
                case P153_STATE_IDLE:
                {
                    // Check if full state transition done
                    // // true: increment FieldCount
                    if (check_full_state_transition())
                    {
                        ++field_count;
                    }
                    
                    // clear state transition array and set [0] <- IDLE
                    reset_state_transition();

                    // if c = '\n'
                    // // next state <- LABEL
                    // // state transition array [1] <- LABEL
                    // else 
                    // // next state <- IDLE
                    if (c == '\n')
                    {
                        nextState = P153_STATE_LABEL;
                        state_transitions[1] = P153_STATE_LABEL;
                    }
                    else
                    {
                        nextState = P153_STATE_IDLE;
                    }
                    break;
                }
                case P153_STATE_LABEL:
                {
                    // if c = '\t'
                    // // next state <- FIELD
                    // // state transition array [2] <- FIELD
                    // else
                    // // update label at field_count 
                    if(c == '\t')
                    {
                        nextState = P153_STATE_FIELD;
                        state_transitions[2] = P153_STATE_FIELD;
                    }
                    else
                    {
                        data_table[field_count][P153_LABEL_IDX] += c;
                    }

                    // Make sure string doesn't exceed maximum length
                    if ( (data_table[field_count][P153_LABEL_IDX]).length() >= P153_MAX_LABEL_LENGTH )
                    {
                        nextState = P153_STATE_FIELD;
                        state_transitions[2] = P153_STATE_FIELD;
                    }
                    
                    break;
                }
                case P153_STATE_FIELD:
                {
                    // dont forget to update data_received_error, fullDataReceived

                    // if c = '\r'
                    // // next state <- IDLE
                    // else
                    // // update field at field_count 
                    if (c == '\r')
                    {
                        nextState = P153_STATE_IDLE;
                        state_transitions[3] = P153_STATE_IDLE;
                    }
                    else
                    {
                        data_table[field_count][P153_FIELD_IDX] += c;
                    }         
                    // Make sure string doesn't exceed maximum length
                    if ( (data_table[field_count][P153_FIELD_IDX]).length() >= P153_MAX_FIELD_LENGTH )
                    {
                        nextState = P153_STATE_IDLE;
                        state_transitions[3] = P153_STATE_IDLE;
                    }

                    // if field count = 47 or data_table[N_FIELDS-1, 0] = "Checksum"
                    // // fullDatareceuved <-true
                    // // add the last character which is the checksum
                    // // save the data table to last table
                    // // reset field count, clear, data table, reset state transition array and set [0] <- IDLE
                    uint8_t final_index = P153_NR_FIELDS-1;
                    if ( (field_count == final_index) || (data_table[final_index][P153_LABEL_IDX] == "Checksum") )
                    {
                        fullDataReceived = true;
                        save_to_last_data_table();
                        length_last_received = field_count;
                        field_count = 0;
                        clear_data_table();
                        reset_state_transition();
                    }
                
                    break;
                }

                default:
                {
                    // clear state transition matrix
                    reset_state_transition();
                    // set next state to IDLE
                    nextState = P153_STATE_IDLE;
                    break;
                }
            } // end case

            currentState = nextState;
        } // end while(available > 0 && !fullSentenceReceived)
    } // end if (easySerial != nullptr) 

    if (fullDataReceived) 
    {
        ++full_data_received;

        fullDataReceived = false;

        char received_checksum = (char)((last_data_table[P153_NR_FIELDS-1][P153_FIELD_IDX]).charAt(0));
        compare_and_reset_checksum( received_checksum );
    }

    return fullDataReceived;
}

bool P153_data_struct::getSentence(String& string) 
{
    int nr_data_labels = get_Nr_User_Labels();
    String user_data[nr_data_labels] = {""};
    // Remove Later
    //String user_labels[4] = {"V","BOOTY","LOAD","Checksum"};

    for(int i = 0; i<nr_data_labels; ++i)
    {
        String label = get_User_Label(i);
        get_Data_Label_and_Field(user_data[i], label);
        //get_Data_Label_and_Field(user_data[i], user_labels[i]);
    }

    get_flattened_data(string, user_data, nr_data_labels);

    if (string.isEmpty()) 
    {
        return false;
    }

    return true;
}

void P153_data_struct::getSentencesReceived(uint32_t& succes, uint32_t& error, uint32_t& length_last) const 
{
    succes      = full_data_received;
    error       = full_data_received_error;
    length_last = length_last_received;
}

void P153_data_struct::setMaxLength(uint16_t maxlenght) 
{
    max_length = maxlenght;
}

// EISH: not sure about this
void P153_data_struct::setLine(uint8_t varNr, const String& line) 
{
    if (varNr < P153_NR_LINES) 
    {
        _lines[varNr] = line;
    }
}

bool P153_data_struct::max_length_reached() const 
{
    if (max_length == 0) { return false; }
    return false;//sentence_part.length() >= max_length;
}

String P153_data_struct::get_User_Label(int idx)
{
        int real_idx = P153_FIRST_USER_LABEL_POS + idx;
        if(real_idx < P153_NR_LINES)
        {
            return _lines[real_idx];
        }
        else
        {
            return "";
        }
}

int P153_data_struct::get_Nr_User_Labels()
{
    return (_lines[P153_NR_USER_LABELS_POS]).toInt();
    //return 4;
}

bool P153_data_struct::check_full_state_transition()
{
  return (state_transitions[0] == P153_STATE_IDLE) && (state_transitions[1] == P153_STATE_LABEL) && (state_transitions[2] == P153_STATE_FIELD) && (state_transitions[3] == P153_STATE_IDLE);
}

void P153_data_struct::reset_state_transition()
{
  state_transitions[0] = P153_STATE_IDLE;
  state_transitions[1] = 0;
  state_transitions[2] = 0;
  state_transitions[3] = 0;  
}

void P153_data_struct::save_to_last_data_table()
{
  for(int row = 0; row < P153_NR_FIELDS; ++row)
  {
    last_data_table[row][P153_LABEL_IDX] = (data_table[row][P153_LABEL_IDX]).substring(0, (data_table[row][P153_LABEL_IDX]).length());
    last_data_table[row][P153_FIELD_IDX] = (data_table[row][P153_FIELD_IDX]).substring(0, (data_table[row][P153_FIELD_IDX]).length());
  }
}

void P153_data_struct::clear_data_table()
{
  for(int row = 0; row < P153_NR_FIELDS; ++row)
  {
    data_table[row][P153_LABEL_IDX] = "";
    data_table[row][P153_FIELD_IDX] = "";
  }
}

void P153_data_struct::get_Data_Label_and_Field(String& user_data, String& user_label)
{
  int golden_row = -1;
  int num_empty_space = 0;

  num_empty_space =  P153_MAX_LABEL_LENGTH - user_label.length();
  user_data = user_label + repeat_char(' ',num_empty_space-1) + ":";

  for (int row = 0; row < P153_NR_FIELDS; row++) 
  {
    if (last_data_table[row][P153_LABEL_IDX] == user_label) 
    {
      golden_row = row;
      break;
    }
  }

  if(golden_row != -1)
  {
    num_empty_space =  P153_MAX_FIELD_LENGTH - (last_data_table[golden_row][P153_FIELD_IDX]).length();
    user_data += last_data_table[golden_row][P153_FIELD_IDX] + repeat_char(' ',num_empty_space);
  }
  else
  {
    user_data += "ERR       ";
  }
  
}String P153_data_struct::repeat_char(char c, int num)
{
  if(num <= 1)
  {
    return "";
  }
  String result = String();
  for(int i=0; i<num; ++i)
  {
    result += c;
  }
  return result;
}

void P153_data_struct::get_flattened_data(String& flattened_data, String* data_list, int num_data_fields)
{
  flattened_data = "";
  for(int i=0; i<num_data_fields; ++i)
  {
    flattened_data += data_list[i];
  }
}

void P153_data_struct::update_checksum(char c)
{
  if (c != '\0')
  {
    last_checksum = (last_checksum + (uint8_t)c);  
  }
}

void P153_data_struct::compare_and_reset_checksum(char received_checksum)
{
  uint8_t checksum = last_checksum-(uint8_t)received_checksum;
  checksum = 256 - checksum;
  if (  ((char)checksum) != received_checksum)
  {
    full_data_received_error = full_data_received_error + 1;
  }
  last_checksum = 0;
}
//V        :12800