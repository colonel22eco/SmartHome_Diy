#include <Arduino.h>

#ifndef TIMER
#define TIMER

/*!
	\brief Klasse für Timer

  Timerklasse, die mit millis() arbeitet, damit kein delay verwendet werden muss
*/
class Timer
{
  typedef unsigned long timer_type;
  
  private:
  timer_type _startTime = 0;    //> Variable zum Speichern von millis() beim starten des Timers
  timer_type _timeout = 0;    //> Variable zum Speichern der Dauer, wie lange der Timer laufen soll

  public:
  //! Memberfunktion
  /*!
    \brief startet den Timer

    \param timeout wie lange der Timer laufen soll
    \return void
  */
  void start(timer_type timeout)
  {
    this->_startTime = millis();
    this->_timeout = timeout;
  }

  //! Memberfunktion
  /*!
    \brief stopt den Timer

    \return void
  */
  void stop()
  {
    this->_timeout = this->_timeout - (millis() - this->_startTime);
  }

  //! Memberfunktion
  /*!
    \brief lässt den Timer nach dem Stoppen weiterlaufen

    \return void
  */
  void resume()
  {
    this->_startTime = millis();
  }

  //! Memberfunktion
  /*!
    \brief getter der verbleibenden Zeit

    \return die verbleibende Zeit bis ablaufen des Timers
  */
  timer_type getRemainingTime()
  {
    return this->_timeout - (millis() - this->_startTime);
  }

  //! Memberfunktion
  /*!
    \brief prüft, ob die eingestellte Zeit abgelaufen ist

    \return true wenn die Zeit abgelaufen ist, false, wenn nicht
  */
  bool isFinished()
  {
    if(millis() - this->_startTime >= this->_timeout)
      return true;

    return false;
  }
};

#endif