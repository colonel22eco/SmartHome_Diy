#include <Arduino.h>

#ifndef RELAIS
#define RELAIS

/*!
	\brief Klasse für RELAIS


*/
class Relais
{
  private:
    int PIN;
    bool state; // true dann relais geschlossen; False dann Relais offen
  public:
    Relais(int GPIO_PIN) : PIN(GPIO_PIN){
        pinMode(this->PIN, OUTPUT);
        state = false;
    } //initiierung des Relais mit dem jeweiligen Pin
  //! Memberfunktion
  /*!
    \brief schaltet das Relais ein

    \return void
  */
  void On()
  {
    digitalWrite(this->PIN, LOW);
    state = true;
  }
    //! Memberfunktion
  /*!
    \brief schaltet das Relais aus

    \return void
  */
  void Off()
  {
    digitalWrite(this->PIN, HIGH);
    state = false;
  }
      //! Memberfunktion
  /*!
    \brief liefert den aktuellen Zustand des Relais

    \return bool
  */
  bool getState()
  {
    return state;
  }

};

#endif