/*------------------------------------------------------------------------------*\
RFG4000 RF Signal Generator implementation
(c,2003 luis-es)

  Coded for AT_SAMD21 (DUE,ItsyBitsy). Works on Micro, Leonardo, etc

  Define this based on hardware wiring
*/
#define DEBUG

#define D_FREQ 4300654321
#define D_PWR   -4
#define D_OUT   1
#define D_ROSC -530
/*
\*------------------------------------------------------------------------------*/

//#include <EEPROM.h>
#include "ADF4351.h"
#include "sSCPI.h"

sSCPI scpi;
ADF4351 sigGen;

double currFreq;
float currPwr,currROsc;

bool serrFLOCK;
int OOK;
uint32_t R[6];
uint32_t heartbeat;

/*
bool ReadEE()
{
int r,a;

  if(EEPROM.read(0)==255)
  {
#ifdef DEBUG
Serial.println("EEPROM empty");
#endif
    return 1;
  }
  for(r=0;r<6;r++)
  {
    a = 16 + r*32;
    R[r]=((uint32_t)EEPROM.read(a) | (uint32_t)EEPROM.read(a+1)<<8 | (uint32_t)EEPROM.read(a+2)<<16 | (uint32_t)EEPROM.read(a+3)<<24 );
#ifdef DEBUG
Serial.print("EEreg_rd");Serial.print(r);Serial.print(": ");Serial.println(R[r],HEX);
#endif
  }
  sigGen.PutREGS(R);
  return 0;
}

bool WriteEE()
{
int r,a;

  sigGen.GetREGS(R);

  EEPROM.write(0,1);

  for(r=0;r<6;r++)
  {
    a = 16 + r*32;
    EEPROM.write(a,(uint8_t)R[r]);EEPROM.write(a+1,(uint8_t)(R[r]>>8));EEPROM.write(a+2,(uint8_t)(R[r]>>16));EEPROM.write(a+3,(uint8_t)(R[r]>>24));
#ifdef DEBUG
Serial.print("EEreg_wr");Serial.print(r);Serial.print(": ");Serial.println(R[r],HEX);
#endif
  }
}
*/

// Set the output frequency
// Parameter is f in Hz 
uint32_t CenterFrequency(double Freq, bool qry)
{
  if(qry)
  {
    SerialPrintDouble(currFreq);Serial.print("\r\n");   //Serial.println((uint64_t)currFreq);
    return 0;
  }
  
#ifdef DEBUG
Serial.print("Set frequency @ ");
SerialPrintDouble(Freq);Serial.print("\r\n");  //Serial.println((uint64_t)Freq);
#endif  

  // range is 35M - 4400M
  if(Freq<35e6 || Freq > 4400e6)// 32-bit high limit at 4295M
  {
    scpi.PushError((char *)"Frequency out of range");
    return 1; // comment just this line to check for frequencies unlocking the PLL
  }  

  //uint64_t iFreq = (uint64_t)Freq;
  if(sigGen.SetFreq(Freq))
  {
    scpi.PushError((char *)"Uncomputable Frequency");
    return 1;
  };
  currFreq=Freq;
  return 0;
}


uint32_t RFPower(double pwr, bool qry)
{
  if(qry)
  {
    Serial.println(currPwr);
    return 0;    
  }

#ifdef DEBUG
Serial.print("Set Power @ ");Serial.println(pwr);
#endif
  if(pwr>5 || pwr<-4)
  {
    scpi.PushError((char *)"Power out of range");
    return 1;
  }
  sigGen.SetPower(pwr);
  currPwr=pwr;
  return 0;
}


// Enable or disable RF output
// Parameter is 1 for enable and 0 for disable
uint32_t SetRFOut(double rfout, bool qry)
{
  bool brfout = (bool)rfout;
  
  sigGen.SetOut(brfout);
  return 0;
}



#if 0
// Start or stop a sweep operation
// Parameter is 1 for start and 0 for stop 
void Sweep(double startSweep)
{
  if (startSweep > 0)
  {
    sweeping = true;
    sweepCurrentf_kHz = sweepStartf_kHz;
  }
  else
    sweeping = false;
}


// Set the start f_kHzuency used for sweeping
// Parameter is the f_kHzuency in Hz
void SetSweepStart(double f_kHzStart)
{
  sweepStartf_kHz = f_kHzStart;
}


// Set the stop f_kHzuency used for sweeping
// Parameter is the f_kHzuency in Hz
void SetSweepStop(double f_kHzStop)
{
  sweepStopf_kHz = f_kHzStop;
}
#endif



uint32_t GetIDN(double, bool qry)
{
  if(qry)
  {
    Serial.println("Mojon City,RFG4000,230001,1.0");
    return 0;
  }

  return 1;
}

// Perform RST
uint32_t DoRST(double na, bool qry)
{
#ifdef DEBUG
  Serial.println("dbg: RST !!");
#endif
  InitParms();
  return 0;
}

// Read the ERROR pool
uint32_t SysError(double na, bool qry)
{
char text[32];

  if(qry)
  {
    scpi.PullError(text);
    Serial.println(text);
    return 0;
  }

  //push error ?
  return 1;
}

uint32_t Impedance(double na, bool qry)
{
  if(qry)
  {
    Serial.println("50");
    return 0;
  }

  //push error ?
  return 1;
}

uint32_t GetAdjRefOsc(double na, bool qry)
{
    Serial.println(currROsc);
    return 0;
}

uint32_t AdjRefOsc(double errHz, bool qry)
{
  if(qry)
  {
    Serial.println(currROsc);
    return 0;
  }

  sigGen.REFin_Err = errHz;
  currROsc=errHz;
  return 0;
}

void InitParms(void)
{
  currPwr=D_PWR;
  currROsc=D_ROSC;
  currFreq=D_FREQ; 
  
  AdjRefOsc(D_ROSC,0);
  CenterFrequency(currFreq,0);
  if(D_OUT)
    SetRFOut(1,0);

}

// ========================================================================================== Initialize
void setup() 
{
  Serial.begin(115200);
#ifdef DEBUG  
  while(!Serial);
#endif

  serrFLOCK = 0;
  OOK=0;
  heartbeat=0;

#ifdef DEBUG
Serial.println("\n\r\r\r\r\r\r\r============================================== SigGen4000 STARTING");
Serial.print("float is ");Serial.println(sizeof(float));
Serial.print("double is ");Serial.println(sizeof(double)); // << check here
Serial.print("uint64_t is ");Serial.println(sizeof(uint64_t));
#ifdef ARM_MATH_CM0PLUS
Serial.println("ARM_MATH_CM0PLUS is defined");
#endif
#endif

  // ---------------------------- Build SCPI command set

  uint8_t grpIDN = scpi.CreateGroup((char *)"*", 0);  // ------------------------- Standard Subsystem 
  scpi.RegisterParameter((char *)"IDN", grpIDN, &GetIDN);
  scpi.RegisterParameter((char *)"RST", grpIDN, &DoRST);
  //scpi.RegisterParameter((char *)"SAV", grpIDN, &WriteEE);
  //scpi.RegisterParameter((char *)"RCL", grpIDN, &ReadEE);
  
  uint8_t grpOutput = scpi.CreateGroup((char *)"OUTP", 0);  // ------------------------- OUTPut Subsystem 
  scpi.RegisterParameter((char *)"", grpOutput, &SetRFOut);
  scpi.RegisterParameter((char *)"IMP", grpOutput, &Impedance);

  uint8_t grpSystem = scpi.CreateGroup((char *)"SYST", 0); // ------------------------- SYSTem Subsystem
  scpi.RegisterParameter((char *)"ERR", grpSystem, &SysError);
  scpi.RegisterParameter((char *)"PRES", grpSystem, &DoRST);
  scpi.RegisterParameter((char *)"PON:TYPE", grpSystem, &DoRST);

  uint8_t grpSource = scpi.CreateGroup((char *)"SOUR", 0); // ------------------------- SOURce Subsystem
  scpi.RegisterParameter((char *)"FREQ", grpSource, &CenterFrequency);
  scpi.RegisterParameter((char *)"FREQ:CW", grpSource, &CenterFrequency);
  scpi.RegisterParameter((char *)"POW", grpSource, &RFPower);

  uint8_t grpRefOsc = scpi.CreateGroup((char *)"ROSC", grpSource);
  scpi.RegisterParameter((char *)"ADJ:VAL", grpRefOsc, &AdjRefOsc);

  // ---------------------------- Initialize SYNTH
  sigGen.Init();

  // ---------------------------- Read stored config
  //ReadEE();

  // ---------------------------- or, set default config
  InitParms();

}

// ============================================================================================ Main loop
void loop() 
{
  // command input check -----------------------
  if (Serial.available() > 0)
    scpi.Parse(Serial.read());

  // PLL lock check ----------------------------
  if(sigGen.FreqLocked()!=true)
  {
    if(serrFLOCK==0)
    {
      delay(4);
      if(sigGen.FreqLocked()!=true)
      {
          serrFLOCK=1;
          scpi.PushError((char *)"PLL Unlock");
      }
    }
  }
  else
  {
    serrFLOCK=0;
  }

#if 0  
  // sweep state operation --------------------
  if (sweeping)
  {
    sweepCurrentf_kHz += sweepStep;
    
    if (sweepCurrentf_kHz < sweepStopf_kHz)
    {
      SetCenterFrequency(sweepCurrentf_kHz);
    }
    else
      sweepCurrentf_kHz = sweepStartf_kHz;
  }
#endif

#if 0
  // OOK operation -------------------------
  ++OOK;
  if(OOK==30000)
  {
      sigGen.SetRFOut(1);
      OOK=-30000;
  }
  if(OOK==0)
    sigGen.SetRFOut(0);

#endif

  // let's blink the LED; of course!
  if(++heartbeat==10000)
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
  if(heartbeat==300000)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    heartbeat=0;
  }


}
