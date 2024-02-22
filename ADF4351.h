/*------------------------------------------------------------------------------*\
Analog Device's ADF4351 Library
(c,2003 luis-es)

  Coded for AT_SAMD21 (DUE,ItsyBitsy). Works on Micro, Leonardo, etc

  Define this based on hardware wiring
*/
#define REF_XTAL 25e6
#define LE_PIN 3
#define LD_PIN 4
/*

  Frequency Limit
  ---------------
  While ADF limit is 4,4 GHz, this value in Hertz exceeds a 32-bit value range.
  On 32-bit compilers, some 64-bit operations can be made, but this limits
  frequency to 4,2949 GHz over 8 or 16-bit uC.

  If frequency is computed in kHz (a tolerable limitation
  for a simple instrument), this limit will rise to 4,29 THz.
	
\*------------------------------------------------------------------------------*/

#include<SPI.h>
#include "sMATH.h"

#define DEBUG

class ADF4351
{
	public:
		// Initialize
		void Init();
		
		// Set the output frequency
		int SetFreq(double freq);
    
		// Enable or disable RF output. 0 = disable, 1 = enable
		void SetOut(uint8_t enabled);
		
		// Set the RF output power
		void SetPower(uint8_t power);

		// Get frequency lock state
		bool FreqLocked();
    
    void GetREGS(uint32_t* reg);
    void PutREGS(uint32_t* reg);
		
		uint32_t REFin;			// Reference oscillator frequency
		int32_t REFin_Err;   // Reference frequency error
			
	private:

		struct Register0
		{
      uint8_t   _n                  : 3;
			uint16_t  Fractional          : 12;
			uint16_t  Integer             : 16;
      bool      res0                : 1;
		};
		
		struct Register1
		{
      uint8_t   _n                  : 3;
			uint16_t  Modulus             : 12;
			uint16_t  PhaseVal            : 12;
			bool      Prescaler           : 1;
			bool      PhaseAdjust         : 1;
      uint8_t   res0                : 3;
		};
		
		struct Register2
		{
      uint8_t   _n                  : 3;
			bool      CounterReset        : 1;
			bool      CPTri               : 1;
			bool      PowerDown           : 1;
			bool      PDPolarity          : 1;
			bool      LDP                 : 1;
			bool      LDF                 : 1;
			uint8_t   ChargePumpCurrent   : 4;
			bool      DoubleBuffer        : 1;
			uint16_t  RCounter            : 10;
			bool      RefDivider          : 1;
			bool      RefDoubler          : 1;
			uint8_t   MUXOut              : 3;
			uint8_t   NoiseMode           : 2;
      bool      res0                : 1;
		};
		
		struct Register3
		{
      uint8_t   _n                  : 3;
			uint16_t  ClockDivider        : 12;
			uint8_t   ClockDividerMode    : 2;
      bool      res0                : 1;
			bool      CSR                 : 1;
      uint8_t   res1                : 2;
			uint8_t   ChargeCancelation   : 1;
			bool      ABP                 : 1;
			bool      BandSelectClockMode : 1;
      uint8_t   res2                : 8;
		};
		
		struct Register4
		{
      uint8_t   _n                  : 3;
			uint8_t   OutputPower         : 2;
			bool      RFOutputEnabled     : 1;
			uint8_t   AuxOutputPower      : 2;
			bool      AuxOutputEnabled    : 1;
			bool      AuxOutputSelect     : 1;
			bool      MTLD                : 1;
			bool      VCOPoweredDown      : 1;
			uint8_t   BandSelectDivider   : 8;
			uint8_t   RFDivider           : 3;
			bool      FBSelect            : 1;
      uint8_t   res0                : 8;
		};
		
		struct Register5
		{
      uint8_t   _n                  : 3;
      uint16_t  res0                : 16;
      uint8_t   res1                : 2;
      bool      res2                : 1;
      uint8_t   LDPinMode           : 2;
      uint8_t   res3                : 8;
		};

		struct Register6          // 8V97051
		{
      uint8_t   _n                  : 3;
      uint8_t   ExtBndSelDiv        : 4;
      bool      res0                : 1;
      uint8_t   res1                : 8;
      uint8_t   band_select_ac      : 2;
      uint8_t   SDMType             : 2;
      bool      ShapeDitherEn       : 1;
      bool      DitherG             : 1;
      uint8_t   SDMOrder            : 2;
      bool      rfouta_hi_pwr       : 1;
      bool      rfoutb_hi_pwr       : 1;
      uint8_t   LDP_Ext             : 2;
      bool      res2                : 1;
      bool      res3                : 1;
      bool      Band_select_do      : 1;
      bool      DigLock             : 1;
		};

		struct Register7          // 8V97051 only
		{
      uint8_t   _n                  : 3;
      bool      SPI_R_WN            : 1;
      uint8_t   Rd_Addr             : 3;
      bool      sclke               : 1;
      uint8_t   ext_fdiv            : 4;
      uint8_t   ext_mod             : 4;
      uint8_t   ext_phase           : 4;
      bool      sel_16b_12b         : 1;
      uint8_t   Dev_ID              : 4;
      uint8_t   Rev_ID              : 3;
      bool      res0                : 1;
      bool      Spi_error           : 1;
      bool      Loss_Anlg_Lock      : 1;
      bool      Loss_Dig_Lock       : 1;      
		};
		

  
    struct Register0 R0;
    struct Register1 R1;
    struct Register2 R2;
    struct Register3 R3;
    struct Register4 R4;
    struct Register5 R5;
    struct Register6 R6;
    struct Register7 R7;

    uint32_t REG[8];

  //uint32_t REG[8]={(uint32_t)&R0,(uint32_t)&R1,(uint32_t)&R2,(uint32_t)&R3,(uint32_t)&R4,(uint32_t)&R5};

		// Build the register to the REG[] array
		void BuildREG(uint8_t num);
		// Write the REG[] array onto the device
		void WriteREG(uint32_t val);

    // build all standard registers
    void BuildAllREG(void);
		// Write all standard registers
		void WriteAllREG(void);

    uint32_t ReadREG(uint32_t val);
		
};

/* Public Functions =============================================================*/
void ADF4351::Init()
{

	pinMode(LE_PIN, OUTPUT);
	pinMode(LD_PIN, INPUT); // INPUT_PULLUP ?
	
	SPI.begin();
	SPI.setDataMode(SPI_MODE0);
	SPI.setBitOrder(MSBFIRST);
	SPI.setClockDivider(SPI_CLOCK_DIV2);	// 16 MHz system clock /2 = 8MHz SPI clock
  
	// Set register 0 default values
  R0._n = 0;
	R0.Integer = 1600;                     // DEFAULT: for 1GHz
	R0.Fractional = 0;                    // DEFAULT: for 1GHz
  R0.res0 = 0;

	// Set register 1 default values
  R1._n = 1;
	R1.PhaseAdjust = 0;
	R1.Prescaler = 1;         // 0 - 4/5(limits to 3.6GHz)    1 - 8/9
	R1.PhaseVal = 1;          // 1 - not used      
	R1.Modulus = 2;                      // DEFAULT: for 1GHz
  R1.res0 = 0;
	
	// Set register 2 default values
  R2._n = 2;
	R2.NoiseMode = 0;         // 0 - Minimize Phase Noise mode // 3 - Minimize Spur mode
	R2.MUXOut = 7;            // 0 - Tri-State // 7 - Enable SDO (for 8V97051)
	R2.RefDoubler = 1;        // it is said this improves noise performance  
	R2.RefDivider = 0;
	R2.RCounter = 20;					// Divide REFin by 20
	R2.DoubleBuffer = 0;
	R2.ChargePumpCurrent = 15; // 7 - depends on loop filter design
	R2.LDF = 0;
	R2.LDP = 0;
	R2.PDPolarity = 1;        // 1 - for passive loop filter
	R2.PowerDown = 0;
	R2.CPTri = 0;             // 0 - normal operation
	R2.CounterReset = 0;      // 0 - normal operation
	R2.res0 = 0;

  // Set register 3 default values
  R3._n = 3;
	R3.ClockDivider = 0;      //150???????????????????
	R3.ClockDividerMode = 0;
	R3.CSR = 0;
	R3.ChargeCancelation = 0;
	R3.ABP = 0;
	R3.BandSelectClockMode = 1;
  R3.res0 = 0;
  R3.res1 = 0;
  R3.res2 = 0;

	// Set register 4 default values
  R4._n = 4;
	R4.FBSelect = 1;             // 1 - feedback from VCO
	R4.RFDivider = 2;            // DEFAULT: for 1GHz
	R4.BandSelectDivider = 25;
	R4.VCOPoweredDown = 0;
	R4.MTLD = 1;                 // mute till LD
	R4.AuxOutputSelect = 0;      // RFoutB not wired on board
	R4.AuxOutputEnabled = 0;     // RFoutB not wired on board
	R4.AuxOutputPower = 0;       // RFoutB not wired on board
	R4.RFOutputEnabled = 0;
	R4.OutputPower = 0;          // min power (supposed -4 dBm)
  R4.res0 = 0;

  // Set register 5 default values;
  R5._n = 5;
  R5.LDPinMode = 1;            // digital lock detect
  R5.res0 = 0;
  R5.res1 = 3;
  R5.res2 = 0;
  R5.res3 = 0;

  // registers 6 and 7 don't need initialization

	REFin = REF_XTAL;
  REFin_Err = 0;
	
	BuildAllREG();
  WriteAllREG();
}


int ADF4351::SetFreq(double freq)
{
float fRES;
double fVCO;
float N,fPFD;
float frac,mod;
int gcd; 

  fRES=1000;
  gcd=-1;
  
  // Calculate Phase Frequency Detector frequency
  fPFD = (REFin+REFin_Err) / (float)R2.RCounter;

	if(R2.RefDoubler==1)
		fPFD *= 2.0;
	if(R2.RefDivider==1)
		fPFD /= 2.0;
#ifdef DEBUG
Serial.print("fPFD: ");Serial.println(fPFD);
#endif
  R4.BandSelectDivider = fPFD/125e3; // 125 kHz is the limit
  
	// Calculate VCO frequency and the correspondent Divider
	if (freq >= 2200e6)
  {
		R4.RFDivider = 0;		// Divide by 1 (VCO fundamental)
    fVCO = freq * 1;
  }
	if (freq < 2200e6)
  {
		R4.RFDivider = 1;		// Divide VCO by 2
    fVCO = freq * 2;
  }
	if (freq < 1100e6)
  {
		R4.RFDivider = 2;		// Divide VCO by 4
    fVCO = freq * 4;
  }
	if (freq < 550e6)
  {
		R4.RFDivider = 3;		// Divide VCO by 8
    fVCO = freq * 8;
  }
	if (freq < 275e6)
  {
		R4.RFDivider = 4;		// Divide VCO by 16
    fVCO = freq * 16;
  }
	if (freq < 137500e3)
  {
		R4.RFDivider = 5;		// Divide VCO by 32
    fVCO = freq * 32;
  }
	if (freq < 68750e3)
  {
		R4.RFDivider = 6;		// Divide VCO by 64
    fVCO = freq * 64;
  }
#ifdef DEBUG
Serial.print("RFDiv: ");Serial.println(R4.RFDivider);
Serial.print("fVCO: ");
  SerialPrintDouble(fVCO);Serial.print("\r\n");   //Serial.println((uint64_t)fVCO);
#endif

	// Calculate the PLL integer divider
  N = fVCO / fPFD;
	R0.Integer = (uint16_t)N;
#ifdef DEBUG
Serial.print("N: ");Serial.println(N);
#endif

	// Calculate MOD & FRAC
  mod  = fPFD / fRES;
  frac = mod * ( N - (double)R0.Integer);

#ifdef DEBUG
Serial.print("VALUES\r\n\tfrac: ");Serial.print(frac);Serial.print(" / mod: ");Serial.println(mod);
#endif

  if(frac==0)
  {
    // ---------------------------------- we're in integer-N Mode
    R0.Fractional=0;
    R1.Modulus=2;
    R2.LDF=1;
    R2.LDP=1;
    R3.ABP=1;
    R3.ChargeCancelation=1;
  }
  else
  {
    // -------------------------------- we're in fractional-N Mode
    mod=round(mod);
    frac=round(frac);
#ifdef DEBUG
Serial.print("ROUNDed\r\n\tfrac: ");Serial.print(frac);Serial.print(" / mod: ");Serial.println(mod);
#endif
    // simplify FRAC/MOD fraction (mod must be < 4096)
    gcd=getGCD(frac,mod);
    if(mod>4095 && gcd==1)
    {
#ifdef DEBUG
Serial.println("*******CAN'T SOLVE!*********");
#endif
      return 1;
    }

    R0.Fractional = frac/gcd;
    R1.Modulus = mod/gcd;
    R2.LDF=0;
    R2.LDP=0;
    R3.ABP=0;
    R3.ChargeCancelation=0;
#ifdef DEBUG
Serial.print("GCD(");Serial.print(gcd);Serial.print(")\r\n\tfrac: ");Serial.print(R0.Fractional);Serial.print(" / mod: ");Serial.println(R1.Modulus);
#endif

  }

#ifdef DEBUG
Serial.print("INT: ");Serial.println(R0.Integer);
Serial.print("FRAC: ");Serial.println(R0.Fractional);
Serial.print("MOD: ");Serial.println(R1.Modulus);
#endif
	
	BuildREG(4);WriteREG(REG[4]);
	BuildREG(3);WriteREG(REG[3]);
	BuildREG(2);WriteREG(REG[2]);
	BuildREG(1);WriteREG(REG[1]);
	BuildREG(0);WriteREG(REG[0]);

#ifdef DEBUG
// let's try to read a register...
Serial.println("Reading R0: ");
uint32_t rREG = ReadREG(0);
Serial.println(rREG,HEX);
#endif

  return 0;
}

void ADF4351::SetOut(uint8_t enable)
{
	
  if(enable)
  {
    //R4.VCOPoweredDown = !enable;
    R4.RFOutputEnabled = enable;
  }
  else
  {
    R4.RFOutputEnabled = enable;
    //R4.VCOPoweredDown = !enable; // eliminates RF leakage, but puts LD down
  }

  BuildREG(4);WriteREG(REG[4]);
}


void ADF4351::SetPower(uint8_t power)
{
int pwr;

	// Calc reg value
  pwr=((power+4)/3);
	
	R4.OutputPower = pwr;

  BuildREG(4);WriteREG(REG[4]);
}


bool ADF4351::FreqLocked()
{
  //if(R4.VCOPoweredDown) // VCO Down trick
  //  return 1;

	return digitalRead(LD_PIN);
}

void ADF4351::GetREGS(uint32_t *reg)
{
int c;

  BuildAllREG();
  
  for(c=0;c<6;c++)
    reg[c]=REG[c];

}

void ADF4351::PutREGS(uint32_t *reg)
{
int c;

  for(c=0;c<6;c++)
    REG[c]=reg[c];

  WriteAllREG();

}

/* Private Functions ============================================================*/

void ADF4351::BuildAllREG()
{
int c;

 	// Write the registers
  for(c=5;c>-1;c--)
  { 
	  BuildREG(c);
  }

}void ADF4351::WriteAllREG()
{
int c;

 	// Write the registers
  for(c=5;c>-1;c--)
  { 
	  WriteREG(REG[c]);
  }

}

void ADF4351::BuildREG(uint8_t num)
{
  switch(num)
  {
    default:
      return;
    case 0:
    	REG[0] = 0x00000000 |
        (uint32_t)(R0.Integer) << 15 |
		    (uint32_t)(R0.Fractional) << 3 ;
      break;
    case 1:
      REG[1] = 0x00000001 |
        (uint32_t)(R1.PhaseAdjust) << 28 |
        (uint32_t)(R1.Prescaler) << 27 |	
        (uint32_t)(R1.PhaseVal) << 15 |
        (uint32_t)(R1.Modulus) << 3 ;
      break;
    case 2:
      REG[2] = 0x00000002 |
        (uint32_t)(R2.NoiseMode) << 29 |
        (uint32_t)(R2.MUXOut) << 26 |
        (uint32_t)(R2.RefDoubler) << 25 |
        (uint32_t)(R2.RefDivider) << 24 |
        (uint32_t)(R2.RCounter) << 14 |
        (uint32_t)(R2.DoubleBuffer) << 13 |
        (uint32_t)(R2.ChargePumpCurrent) << 9 |
        (uint32_t)(R2.LDF) << 8 |
        (uint32_t)(R2.LDP) << 7 |
        (uint32_t)(R2.PDPolarity) << 6 |
        (uint32_t)(R2.PowerDown) << 5 |
        (uint32_t)(R2.CPTri) << 4 |
        (uint32_t)(R2.CounterReset) << 3 ;
      break;
    case 3:
      REG[3] = 0x00000003 |
        (uint32_t)(R3.res2) << 24 |
        (uint32_t)(R3.BandSelectClockMode) << 23 |
        (uint32_t)(R3.ABP) << 22 |
        (uint32_t)(R3.ChargeCancelation) << 21 |
        (uint32_t)(R3.res1) << 19 |
        (uint32_t)(R3.CSR) << 18 |
        (uint32_t)(R3.res0) << 17 |
        (uint32_t)(R3.ClockDividerMode) << 15 |
        (uint32_t)(R3.ClockDivider) << 3 ;
      break;
    case 4:
      REG[4] = 0x00000004 |
        (uint32_t)(R4.FBSelect) << 23 |
        (uint32_t)(R4.RFDivider) << 20 |
        (uint32_t)(R4.BandSelectDivider) << 12 |
        (uint32_t)(R4.VCOPoweredDown) << 11 |
        (uint32_t)(R4.MTLD) << 10 |
        (uint32_t)(R4.AuxOutputSelect) << 9 | 
        (uint32_t)(R4.AuxOutputEnabled) << 8 | 
        (uint32_t)(R4.AuxOutputPower) << 6 | 
        (uint32_t)(R4.RFOutputEnabled) << 5 | 
        (uint32_t)(R4.OutputPower) << 3 ;
      break;
    case 5:
      REG[5] = 0x00000005 |
        (uint32_t)(R5.LDPinMode) << 22 |
        (uint32_t)(R5.res1) << 19 ;
        break;
    case 6:
      REG[6] = 0x00000006 |
        (uint32_t)(R6.ExtBndSelDiv) << 3 |
        (uint32_t)(R6.res0) << 7 |
        (uint32_t)(R6.res1) << 8 |
        (uint32_t)(R6.band_select_ac) << 16 |
        (uint32_t)(R6.SDMType) << 18 |
        (uint32_t)(R6.ShapeDitherEn) << 20 |
        (uint32_t)(R6.DitherG) << 21 |
        (uint32_t)(R6.SDMOrder) << 22 |
        (uint32_t)(R6.rfouta_hi_pwr) << 24 |
        (uint32_t)(R6.rfoutb_hi_pwr) << 25 |
        (uint32_t)(R6.LDP_Ext) << 26 |
        (uint32_t)(R6.res2) << 28 |
        (uint32_t)(R6.res3) << 29 |
        (uint32_t)(R6.Band_select_do) << 30 |
        (uint32_t)(R6.DigLock) << 31;
        break;
    case 7:
      REG[7] = 0x00000007 |
        (uint32_t)R7.sclke << 7 |
        (uint32_t)R7.Rd_Addr << 4 |
        (uint32_t)R7.SPI_R_WN << 3 ;
        break;
  }
}

void ADF4351::WriteREG(uint32_t val)
{

  digitalWrite(LED_BUILTIN, HIGH);
#ifdef DEBUG
Serial.print("\tw");Serial.print(val&7);Serial.print(": ");Serial.println(val,HEX);
#endif

	val=__builtin_bswap32((uint32_t)val);
	
  digitalWrite(LE_PIN, LOW);
  SPI.transfer(&val,4);
  digitalWrite(LE_PIN, HIGH);

  digitalWrite(LED_BUILTIN, LOW);

}

uint32_t ADF4351::ReadREG(uint32_t val)
{
uint32_t rreg;

  R7.Rd_Addr = val;
  R7.SPI_R_WN = 1;
  //R7.sclke = 1;

  BuildREG(7);
#ifdef DEBUG
Serial.print("\tw");Serial.print((uint32_t)REG[7]&7);Serial.print(": ");Serial.println(REG[7],HEX);
#endif

	rreg=__builtin_bswap32((uint32_t)REG[7]);
	
  digitalWrite(LE_PIN, LOW);
  SPI.transfer(&rreg,4);
  digitalWrite(LE_PIN, HIGH);

#ifdef DEBUG
Serial.print("\tr");Serial.print((uint32_t)rreg&7);Serial.print(": ");Serial.println(rreg,HEX);
#endif

  rreg=__builtin_bswap32((uint32_t)rreg);

  return rreg;
  
}
