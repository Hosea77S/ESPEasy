// Add relevant include files
# include "_Plugin_Helper.h"

// Add relevent #defines 
#define PLUGIN_153
#define PLUGIN_ID_153 153           // plugin id
#define PLUGIN_NAME_153 "Hosea"

// Add relevant global variables

boolean Plugin_153(uint8_t function, struct EventStruct *event, String& string)
{
	boolean success = false;
	switch(function)
	{
		case PLUGIN_DEVICE_ADD:
		{
			break;
		}
	
		case PLUGIN_GET_DEVICENAME:
		{
			break;
		}
	
		case PLUGIN_GET_DEVICEVALUENAMES:
		{
			success = true;
			break;
		}
	
		case PLUGIN_WEBFORM_LOAD:
		{
			success = true;
			break;
		}
	
		case PLUGIN_WEBFORM_SAVE:
		{
			success = true;
			break;
		}
	
		case PLUGIN_INIT:
		{
			success = true;
			break;
		}
	
		case PLUGIN_SERIAL_IN:
		{
			success = true;
			break;
		}
	
		case PLUGIN_READ:
		{
			if(plugin_153_init)
			{
				success=true;
			}
			break;
		}
	
		case PLUGIN_WRITE:
		{
			if(some_conditions_met)
			{
				success = true;
			}
			break;
		} 
	} // end case
}