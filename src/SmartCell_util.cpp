// SmartCell_util.cpp

#include "SmartCell_util.h"

/**
 * @brief Devuelve true si la hora se sincronizo.
 * 
 * verifica que el año actual es mayor a 2018.
 * 
 * @return true 
 * @return false 
 */
bool timeIsSynced()
{
  return (time(NULL) > 1514764800); // 1514764800 = UNIX timestamp para 1/1/2018 00:00:00
}

/**
 * @brief Sincroniza la hora usando la red LoRa.
 * 
 * @note Utiliza el comando MAC devTimeReq.
 * 
 * @param attempts Maxima cantidad de intentos de sincronizacion.
 * @param timezone Zona horaria.
 * @return true - La sincronizacion se realizo con exito.
 */
bool syncTime(const uint8_t attempts, const float timeZone)
{
  //! getGPSTime siempre el UTC sin importar la hora del gateway
  // set_time((dot->getGPSTime() / 1000) + 315964800)

  for (uint8_t i = 0; i < attempts; i++)
  {
    // realizar intenito.
    uint64_t recievedTimestamp = dot->getGPSTime();
    // verificar
    if (recievedTimestamp > 1198800018000) // 1198800018 = GPS timestamp para 1/1/2018 00:00:00
    {
      // Diferencia (en 2020) entre UNIX epoch con GPS epoch 315964782
      recievedTimestamp = (recievedTimestamp / 1000) + (315964782 + static_cast<uint64_t>(timeZone * 3600));
      set_time(recievedTimestamp);
      time_t currentTimestamp = time(NULL);
      logInfo("Clock Sync to %s", ctime(&currentTimestamp));
      return true;
    }
  }
  // reportar error si no sincronizo
  logError("Clock didn't sync");
  return false;
}

/**
 * @brief Devuelve unicamente el hora actual (0 - 23).
 * 
 * @return uint8_t (0 - 23)
 */
uint8_t getHour()
{
  // obtener la cantidad de horas
  return ((static_cast<uint32_t>(time(NULL)) / 3600) % 24);
}

/**
 * @brief Setea el RTC a una hora especifica
 * 
 * @param timestamp Segundos desde el UNIX epoch
 */
void setManualTime(const uint32_t timestamp)
{
  set_time(timestamp);
}

/**
 * @brief Imrime por consola el tiempo actual.
 * 
 * @note Utiliza nivel de log INFO
 */
void printTime()
{
  time_t currentTimestamp = time(NULL);
  logInfo("Current Time: %s", ctime(&currentTimestamp));
}

//------------------------------------------------------------------

/**
 * @brief Envia dimming, power y energy
 * 
 * @param dimming valor de dimming (0 - 1)
 * @param power valor de potencia (>0)
 * @param energy valor de potencia (>0)
 */
void send_lightStatus(const float dimming, const float power, const float energy)
{
  // Solo ejecutar si se esta unido a la red lora
  if (dot->getNetworkJoinStatus())
  {
    // buffer a enviar
    std::vector<uint8_t> tx_data;

    // cargar el buffer con los datos
    tx_data.push_back('S');

    uint16_t aux16 = static_cast<uint16_t>(power);
    tx_data.push_back(static_cast<uint8_t>(aux16 >> 8));
    tx_data.push_back(static_cast<uint8_t>(aux16));

    tx_data.push_back(static_cast<uint8_t>(dimming * 100));

    aux16 = static_cast<uint16_t>(energy);
    tx_data.push_back(static_cast<uint8_t>(aux16 >> 8));
    tx_data.push_back(static_cast<uint8_t>(aux16));

    // enviar el buffer
    send_data(tx_data);
  }
}

/**
 * @brief Enviar las configuraciones actuales del nodo.
 * 
 * (modo, valor de dimming manual, retardo entre ciclos)
 * 
 * @param lightMode Modo de funcionamiento.
 * @param manualDimming Nivel de dimming manual.
 * @param loopDelay Retardo entre ciclos [segundos]
 */
void send_smartCellConfig(const uint8_t lightMode, const uint8_t manualDimming, const uint16_t loopDelay)
{
  // Solo ejecutar si se esta unido a la red lora
  if (dot->getNetworkJoinStatus())
  {
    // buffer a enviar
    std::vector<uint8_t> tx_data;

    // cargar el buffer con los datos
    tx_data.push_back('E');

    tx_data.push_back(lightMode);
    tx_data.push_back(manualDimming);
    tx_data.push_back(static_cast<uint8_t>((loopDelay & 0xFF00) >> 8));
    tx_data.push_back(static_cast<uint8_t>(loopDelay & 0x00FF));

    // enviar el buffer
    send_data(tx_data);
  }
}

/**
 * @brief Enviar el timestamp actual.
 * 
 */
void send_currentTime()
{
  // Solo ejecutar si se esta unido a la red lora
  if (dot->getNetworkJoinStatus())
  {
    // obtener timestamp
    time_t currentTimestamp = time(NULL);

    // buffer a enviar
    std::vector<uint8_t> tx_data;

    // cargar datos en el buffer
    tx_data.push_back('H');

    tx_data.push_back(static_cast<uint8_t>((currentTimestamp & 0xFF000000) >> 24));
    tx_data.push_back(static_cast<uint8_t>((currentTimestamp & 0x00FF0000) >> 16));
    tx_data.push_back(static_cast<uint8_t>((currentTimestamp & 0x0000FF00) >> 8));
    tx_data.push_back(static_cast<uint8_t>(currentTimestamp & 0x000000FF));

    // enviar el buffer
    send_data(tx_data);
  }
}