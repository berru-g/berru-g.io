// CAPACITIVE TOUCH BASIC AVEC TESTE POTARD EN ANALOG MARS 2020
// BASE DIFFERENTE EN BAULT | CAPACITIVE A 115200 ET POTARD A 9600 |Pas encore trouvé la bonne structure


#include <ADCTouchSensor.h>

#ifdef ADCTOUCH_INTERNAL_GROUNDING
# define GROUNDED_PIN -1
#endif

#ifdef ARDUINO_ARCH_AVR
# define SLOW_SAMPLING
# undef FAST_SAMPLING
# undef MEDIUM_SAMPLING


#define LED_OFF        0

#if defined(PIN_A7)
int pins[] = {A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15};
#else
int pins[] = {A0,A1,A2,A3,A4,A5};
#endif

#else // STM32F1

#define USB_MIDI

#include <USBMIDI.h> // https://github.com/arpruss/USBHID_stm32f1

#define LED_BUILTIN PB12 //
#define LED_OFF        1
int pins[] = {PA0,PA1,PA2,PA3,PA4,PA5,PA6,PA7};

#ifndef ADCTOUCH_INTERNAL_GROUNDING
# define ADCTOUCH_INTERNAL_GROUNDING PA8
#endif

USBMIDI MidiUSB;

#endif

////NOTE BLANCHE|||||PAS DE BEMOL||||AJOUTER 5 BROCHE ANALOG PAR GAMME
const uint8_t notes[] = {52,54,56,57,59,61,63,64,66,68,69,71,73,75}; //GAMME COMPLETE {64, 65,66, 67,68, 69,70, 71, 72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87};
const int numPins = sizeof(pins)/sizeof(*pins);
const uint8_t NOTE_ON = 0b10010000;
const uint8_t NOTE_OFF =0b10000000;
int ref[numPins];
uint8_t prev[numPins];
ADCTouchSensor* sensors[numPins];
int val = 0; 
void setup() 
{
#if defined(ARDUINO_ARCH_AVR)
// default: divide clock by 128
#ifdef FAST_SAMPLING
   ADCSRA = (ADCSRA & ~0b111) | 0b100; //  16
#else
#ifdef MEDIUM_SAMPLING
   ADCSRA = (ADCSRA & ~0b111) | 0b110; // 64
#endif
#endif
#endif

#ifndef USB_MIDI
    Serial.begin(115200);
#else
	MidiUSB.begin();
#endif	

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, 1^LED_OFF);     

    for (int i=0; i<numPins; i++) {
        sensors[i] = new ADCTouchSensor(pins[i]);
        sensors[i]->begin();
        prev[i] = 0;
    }

    digitalWrite(LED_BUILTIN, 0^LED_OFF);

//    uint32_t t = micros();
//    ADCTouch.read(A0, 10000);
//    t = micros() -t;
//    Serial.println(t);
} 

void midiNote(uint8_t status, uint8_t note, uint8_t velocity) {
#ifdef USB_MIDI
  if (status == NOTE_ON)
      MidiUSB.sendNoteOn(0, note, velocity);
  else if (status == NOTE_OFF)
      MidiUSB.sendNoteOff(0, note, velocity);    
#else
  Serial.write(status);
  Serial.write(note);
  Serial.write(velocity);
#endif
}

void loop() 
{
    uint8_t pressed = 0;

    for (int i=0; i<numPins; i++) {
      if (sensors[i]->read() > 5) {
         pressed = 1;
         if(!prev[i]) {
           midiNote(NOTE_ON, notes[i], 127);
           prev[i] = 1;
         }
      }
      else {
         if(prev[i]) {
           midiNote(NOTE_OFF, notes[i], 127);
           prev[i] = 0;
         }
      }
    }
    digitalWrite(LED_BUILTIN, pressed^LED_OFF);
}
