/*
Versione Arduino dell'esempio Hunter con i2S alposto di un DAC
maurizio.conti@fablabromagna.org
*/

#include <I2S.h>

// Istanzia un i2S in uscita
I2S i2s(OUTPUT);

// su questi pin 
#define pBCLK 26
#define pWS (pBCLK+1)
#define pDOUT 28

////////////////////////////////////////
//
// Hunter
//
////////////////////////////////////////

//DDS parameters
#define two32 4294967296.0 // 2^32 
#define Fs 40000

// the DDS units:
volatile unsigned int phase_accum_main;
volatile unsigned int phase_incr_main = (800.0*two32)/Fs ;//

// SPI data
uint16_t DAC_data ; // output value

// DDS sine table
#define sine_table_size 256
volatile int sin_table[sine_table_size] ;
struct repeating_timer timer;

// Timer ISR
bool repeating_timer_callback(struct repeating_timer *t)
{ 
  // DDS phase and sine table lookup
  phase_accum_main += phase_incr_main;

  DAC_data = (((sin_table[phase_accum_main>>24] + 2048) & 0xffff)) >> 2;

  // Qui Hunter usava una scrittura su SPI per andare verso il DAC
  // spi_write16_blocking(SPI_PORT, &DAC_data, 1) ;
  i2s.write(DAC_data);

  // Usato per debug con oscilloscopio per vedere la frequenza 
  // di scansione (40KHz, vedi variabile Fs)
  digitalWrite(18, !digitalRead(18));

  return true;
}

////////////////////////////////////////
// Fine codice Hunter
////////////////////////////////////////


void setup() {

  // Pin usato per debug hardware
  pinMode(18, OUTPUT);

  // Settaggio dei pin della i2S
  i2s.setBCLK(pBCLK);
  i2s.setDATA(pDOUT);
  i2s.setBitsPerSample(16);

  // inizializzazione della porta i2S
  if (!i2s.begin(Fs)) {
    Serial.println("Errore nella i2S!");
    while (1); // fermo qui
  }
  
  // Inizio Hunter
  // === build the sine lookup table =======
  // scaled to produce values between 0 and 4096
  int ii;
  for (ii = 0; ii < sine_table_size; ii++){
       sin_table[ii] = (int)(2047*sin((float)ii*6.283/(float)sine_table_size));
  }

  // Negative delay so means we will call repeating_timer_callback, and call it again
  // 25us (40kHz) later regardless of how long the callback took to execute
  add_repeating_timer_us( -25, repeating_timer_callback, NULL, &timer);

  //
  // fine Hunter

}

void loop() 
{
  // Tutto gira via irq, qui non serve nulla...
}
