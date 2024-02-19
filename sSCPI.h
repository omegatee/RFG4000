/* ------------------------------------------------------------------------------*\
Simple SCPI Parser
(c,2003 luis-es)

  Coded for Arduino ATMEGA 32U4 (Micro, Leonardo, etc)

  Define this based on data size needed and available memory
*/
#define PARAM_MAX		16
#define GROUP_MAX		16
#define CMD_LEN_MAX	32
#define ERR_MAX     8
/*


\* ----------------------------------------------------------------------------- */

class sSCPI
{
	public:
		// Define callback function pointer type
		typedef uint32_t (*func_t)(double,bool);
		
		sSCPI();
		
		uint8_t CreateGroup(char* name, uint8_t parent);
		uint8_t RegisterParameter(char* command, uint8_t group, func_t function);
		void Parse(char byte);
    void PushError(char* name);
    void PullError(char* message);


	private:
		
		struct GroupType
		{
			uint8_t id;
			uint8_t parentId;
			const char* name;
      //uint8_t len;
		};
		
		struct ParameterType 
		{
			uint8_t id;			// Parameter ID. Should be larger than 0, value of 0 means unused.
			uint8_t groupId;
			char* name;
			func_t function;
		};
		
		uint8_t grpIndex;
		GroupType groups[GROUP_MAX];
		ParameterType Parameters[PARAM_MAX];
    char buffS[CMD_LEN_MAX];
		uint8_t buffSidx;
		uint8_t buffSptr;
    uint8_t errIndex;
    const char *ErrorMessage[ERR_MAX][CMD_LEN_MAX];

		const char* GetGroupName(uint8_t index);
		uint8_t GetGroupID(char* name);
		uint8_t GetCommandID(char* name);
    bool scanGroup(char* string, char *groupgot);
    void scanCommand(char* string, char *commandgot);
    void scanValue(char* string, char *paramgot);

};


sSCPI::sSCPI()
{
	grpIndex = 1;
	buffSidx = 0;
  buffSidx = 0;

  errIndex = 0;
  *ErrorMessage[0]=(char*)"No error";
}

/* Public Functions =============================================================*/

uint8_t sSCPI::CreateGroup(char* name, uint8_t parent)
{
	groups[grpIndex].id = grpIndex;
	groups[grpIndex].name = name;
	
	if(grpIndex<GROUP_MAX)
    grpIndex++;
	
	return grpIndex - 1;
}


uint8_t sSCPI::RegisterParameter(char* command, uint8_t group, func_t function)
{
	uint8_t newId = 1;
	
	// Find first available id that can be used
	for (int i = 0; i < PARAM_MAX; i++)
		if (Parameters[i].id == newId)
		{
			newId++;
			i = 0;
		}
		
	for (int i = 0; i < PARAM_MAX; i++)
		if (Parameters[i].id == 0)
		{
			Parameters[i].id = newId;
			Parameters[i].groupId = group;
			Parameters[i].name = command;
			Parameters[i].function = function;
			break;
		}
	
	return newId;
}

void sSCPI::Parse(char c)
{
	buffS[buffSidx] = c;
	buffSidx++;
  bool q=0;
  
	if ((c == '\r') || (c == '\n') || (c == ';'))
	{
		char group[10];
		char command[16];
		char paramValue[12];

    buffS[buffSidx] = 0;

    // cleanUp buffS
    int s=c=0;
    while( (buffS[s]==' ' || buffS[s]==':') && buffS[s] )
      s++;

    while( (buffS[s]!='\r' && buffS[s]!='\n' && buffS[s]!=';') && buffS[s] )
      buffS[c++] = buffS[s++];

    buffS[c]=0;
    buffSptr=0;
#ifdef DEBUG       
Serial.print("dbg: -------------------[");Serial.print(buffS);Serial.println("]");
#endif
    group[0]=0;
    scanGroup(buffS,group);
#ifdef DEBUG       
Serial.print("dbg:group[");Serial.print(group);Serial.println("]");
#endif
    command[0]=0;
    scanCommand(buffS,command);
#ifdef DEBUG       
Serial.print("dbg:command[");Serial.print(command);Serial.println("]");
#endif
    paramValue[0]=0;
    scanValue(buffS,paramValue);
#ifdef DEBUG       
Serial.print("dbg:param[");Serial.print(paramValue);Serial.println("]");
#endif


		uint8_t grpId = GetGroupID(group);
		uint8_t cmdId = GetCommandID(command);
			
		if (cmdId > 0)
		{
			for (int i = 0; i < PARAM_MAX; i++)
      {
				if ((Parameters[i].groupId == grpId) && (Parameters[i].id == cmdId))
				{	
  				// evaluate parameter
          double v;
					if(!strncmp(paramValue,"ON",2))
						v = 1;
					else if(!strncmp(paramValue,"OF",2))
						v = 0;
					else
						v = strtod(paramValue, NULL);

          if(!strcmp(paramValue,"?"))
            q=1;

          Parameters[i].function(v,q);
				}
      }
		}	
		else
    {
      PushError((char *)"Invalid parameter"); // enqueue error
    }
		
		// Reset commands builder index
		buffSidx = 0;
    //buffS[0]=0;
	}
}

void sSCPI::PushError(char* name)
{
char text[32];

  if(errIndex<ERR_MAX)
    ++errIndex;

  *ErrorMessage[errIndex]=(char*)name;
#ifdef DEBUG
Serial.print("***[ERROR] set to ");Serial.println(*ErrorMessage[errIndex]);
#endif
}

void sSCPI::PullError(char* message)
{
  sprintf((char*)message,"+%d,\"%s\"",errIndex,*ErrorMessage[errIndex]);

  if(errIndex!=0)
    errIndex--;

}

/* Private Functions ============================================================*/

const char* sSCPI::GetGroupName(uint8_t index)
{
	return groups[index].name;
}


uint8_t sSCPI::GetGroupID(char* name)
{
int i;

	for (i = 0; i < GROUP_MAX; i++)
  {
		//if (strncmp(name, groups[i].name, groups[i].len) == 0)
    //if (strcmp(name, groups[i].name) == 0)
    if (strncmp(name, groups[i].name, strlen(groups[i].name)) == 0)
    {
			return groups[i].id;
    }
  }	
  PushError((char *)"Undefined header");
	return 0;
}


uint8_t sSCPI::GetCommandID(char* name)
{
int i;

	for (i = 0; i < PARAM_MAX; i++)
  {
		if (strcmp(name, Parameters[i].name) == 0)
    {
			return Parameters[i].id;
    }
  }
  PushError((char *)"Unknown command");	
	return 0;
}


bool sSCPI::scanGroup(char *string, char *groupgot)
{
int s,c;

#ifdef DEBUG
Serial.print("scGrp>>");Serial.println(&string[buffSptr]);
#endif
  s=0;c=0;

  if(string[buffSptr]=='*')
  {
    groupgot[c]='*';groupgot[++c]=0;
    //string += sizeof(char);
    buffSptr=c;
    return true;
  }
  else
  {
     while( /*string[buffSptr]!=' ' &&*/ string[buffSptr]!=':' && 
            (string[buffSptr]!='\r' && string[buffSptr]!='\n' && string[buffSptr]) )
        groupgot[c++]=string[buffSptr++];
    groupgot[c]=0;
    //string += (sizeof(char)*s);
    buffSptr++;
    //buffSptr=s;
    return true;   
  }
  return false;
}

void sSCPI::scanCommand(char* string, char *commandgot)
{
int s,c;

#ifdef DEBUG
Serial.print("scCmd>>");Serial.println(&string[buffSptr]);
#endif
  s=c=0;

  if(string[buffSptr]==' ')
  {
    commandgot[c]=0;
    buffSptr++;
    return;
  }

  while( string[buffSptr]!=' ' && string[buffSptr]!='?' &&
        (string[buffSptr]!='\r' && string[buffSptr]!='\n' && string[buffSptr]) )
    commandgot[c++]=string[buffSptr++];
  commandgot[c]=0;
  //string += sizeof(char)*c;
  buffSptr++;
  //buffSptr=s;
}

void sSCPI::scanValue(char* string, char *paramgot)
{
int s,c;

#ifdef DEBUG
Serial.print("scVal>>");Serial.println(&string[buffSptr]);
#endif
  s=c=0;

//    while((string[buffSptr]!=' ' || string[buffSptr]!='?') && string[buffSptr])
//     buffSptr++;

    if(string[buffSptr]=='?')
      buffSptr++;
    else
      buffSptr--;

    while( ( string[buffSptr]!='\r' || string[buffSptr]!='\n' || string[buffSptr]!=':') && string[buffSptr] )
      paramgot[c++]=string[buffSptr++];

    paramgot[c]=0;
}


