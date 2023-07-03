#define PLUGINSTRUCTS_P153_DATA_STRUCT_H

# include "../../_Plugin_Helper.h"
# include <ESPeasySerial.h>

// For Custom Task settings
# define P153_NR_USER_LABELS_POS    0
# define P153_FIRST_USER_LABEL_POS  6
# define P153_MAX_NR_USER_LABELS    4
# define P153_NR_LINES              (P153_FIRST_USER_LABEL_POS + P153_MAX_NR_USER_LABELS)
// For Webforms
# define P153_NR_FORM_chars         10 // used for num chars in textbox
// For State Machine
# define P153_STATE_IDLE            1
# define P153_STATE_LABEL           2
# define P153_STATE_FIELD           3
# define P153_MAX_LABEL_LENGTH      10
# define P153_MAX_FIELD_LENGTH      10
# define P153_NR_STATE_TRANS        4
# define P153_NR_FIELDS             48 //MUST CHANGE
# define P153_LABEL_IDX             0
# define P153_FIELD_IDX             1

struct P153_data_struct : public PluginTaskData_base 
{
public:
    P153_data_struct() = default;

    virtual ~P153_data_struct();

    void reset();

    bool init(  ESPEasySerialPort port,
                const int16_t   serial_rx,
                const int16_t   serial_tx,
                unsigned long   baudrate,
                uint8_t         config);

    // Called after loading the config from the settings.
    // Will interpret some data and load caches.

    bool isInitialized() const;

    void sendString(const String& data);

    void sendData(  uint8_t *data,
                    size_t   size);

    bool loop();

    // Get the received sentence
    // @retval true when the string is not empty.
    bool getSentence(String& string);

    void getSentencesReceived(  uint32_t& succes,
                                uint32_t& error,
                                uint32_t& length_last) const;//change

    void setMaxLength(uint16_t maxlenght); 

    // EISH: 
    void setLine(uint8_t varNr,
                const String& line);

    // get label stored in custom configs
    String get_User_Label(int idx);

    int get_Nr_User_Labels();

    // EISH:
    // Made public so we don't have to copy the values when loading/saving.
    String _lines[P153_NR_LINES];

private:

    bool max_length_reached() const; // change
    bool check_full_state_transition();   
    void reset_state_transition();
    void save_to_last_data_table();
    void clear_data_table();
    void get_Data_Label_and_Field(String& user_data, String& user_label);
    String repeat_char(char c, int num);
    void get_flattened_data(String& flattened_data, String* data_list, int num_data_fields);
    void update_checksum(char c);
    void compare_and_reset_checksum(char received_checksum);
    

    ESPeasySerial *easySerial =                         nullptr;
    String         data_table[P153_NR_FIELDS][2] =      {{""}};
    String         last_data_table[P153_NR_FIELDS][2] = {{""}};
    String         last_sentence =                      {""}; // might not need 
    uint16_t       max_length =                         {20}; //Change
    uint8_t        field_count =                        {0};
    uint32_t       full_data_received =                 {0};
    uint32_t       full_data_received_error =           {0};
    uint32_t       length_last_received =               {0}; //change
    uint8_t        currentState =                       P153_STATE_IDLE;
    uint8_t        nextState =                          P153_STATE_IDLE;
    uint8_t        state_transitions[P153_NR_STATE_TRANS] = {0}; 
    uint8_t        last_checksum =                      {0};
};