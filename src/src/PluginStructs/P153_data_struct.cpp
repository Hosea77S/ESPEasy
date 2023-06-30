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
    bool fullSentenceReceived = false;

    if (easySerial != nullptr) 
    {
        int available = easySerial->available();

        while (available > 0 && !fullSentenceReceived) 
        {
            // Look for end marker
            char c = easySerial->read();
            --available;

            if (available == 0) 
            {
                available = easySerial->available();
                delay(0);
            }

            switch (currentState) 
            {
                case P153_S0:
                {
                    if (c == '\n')
                        nextState = P153_S1;
                    else
                        nextState = P153_S0;
                    break;
                }
                case P153_S1:
                {
                    if (c == 'V')
                        nextState = P153_S2;
                    else
                        nextState = P153_S0;
                    break;
                }
                case P153_S2:
                {
                    if (c == '\t')
                        nextState = P153_S3;
                    else
                        nextState = P153_S0;
                    break;
                }
                case P153_S3:
                    if (c == '\r') 
                    {
                        nextState = P153_S0;
                        const size_t length = sentence_part.length();
                        bool valid = length > 0;

                        for (size_t i = 0; i < length && valid; ++i)
                        {
                            if((sentence_part[i] > 127) || (sentence_part[i] < 9))
                            {
                                sentence_part = String();
                                ++sentences_received_error;
                                valid = false;
                            }
                        }

                        if(valid)
                        {
                            fullSentenceReceived = true;
                            last_sentence = sentence_part;
                            sentence_part = String();
                        }
                    }  
                    else 
                    {
                        sentence_part += c;
                    }
                    break;

                default:
                {
                    sentence_part += c;
                    break;
                }
            } // end case

            if (max_length_reached()) 
            { 
                fullSentenceReceived = true; 
            }
            currentState = nextState;
        } // end while(available > 0 && !fullSentenceReceived)
    } // end if (easySerial != nullptr) 

    if (fullSentenceReceived) 
    {
        ++sentences_received;
        length_last_received = last_sentence.length();
    }

    return fullSentenceReceived;
}

bool P153_data_struct::getSentence(String& string) 
{
    string = last_sentence;

    if (string.isEmpty()) 
    {
        return false;
    }
    last_sentence = String();
    return true;
}

void P153_data_struct::getSentencesReceived(uint32_t& succes, uint32_t& error, uint32_t& length_last) const 
{
    succes      = sentences_received;
    error       = sentences_received_error;
    length_last = length_last_received;
}

void P153_data_struct::setMaxLength(uint16_t maxlenght) 
{
    max_length = maxlenght;
}

// EISH: not sure about this
void P153_data_struct::setLine(uint8_t varNr, const String& line) 
{
    if (varNr < P153_NR_lines) 
    {
        _lines[varNr] = line;
    }
}

bool P153_data_struct::max_length_reached() const 
{
    if (max_length == 0) { return false; }
    return sentence_part.length() >= max_length;
}
