#define PLUGINSTRUCTS_P153_DATA_STRUCT_H

# include "../../_Plugin_Helper.h"
# include <ESPeasySerial.h>

# define P153_FIRST_LABEL_POS    6
# define P153_NR_FIELD_LABELS    4
# define P153_NR_lines           (P153_FIRST_LABEL_POS + P153_NR_FIELD_LABELS)
# define P153_NR_FORM_chars      10 // used for num chars in textbox
# define P153_S0                 0
# define P153_S1                 1
# define P153_S2                 2
# define P153_S3                 3

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
    // EISH:
    // Made public so we don't have to copy the values when loading/saving.
    String _lines[P153_NR_lines];

private:

    bool max_length_reached() const;

    ESPeasySerial *easySerial = nullptr;
    String         sentence_part;
    String         last_sentence;
    uint16_t       max_length               = 20; //Change
    uint32_t       sentences_received       = 0;
    uint32_t       sentences_received_error = 0;
    uint32_t       length_last_received     = 0; //change
    uint8_t        currentState = P153_S0;
    uint8_t        nextState = P153_S0;
};