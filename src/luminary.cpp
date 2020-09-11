#include "dot_util.h"
// #include "RadioEvent.h"
#include "EventSlot.h"

#if ACTIVE_EXAMPLE == LUMINARY

// [START] Luminary Includes

#include "CurrentSensor.h"
#include "DimmingCurves.h"
#include "PhotoCell.h"
#include "LightController.h"
#include "LightOutput.h"
#include "DimmingDemo.h"

#include "SmartCell_util.h"

#include "LedHandler.h"

// [END] Luminary Includes

/////////////////////////////////////////////////////////////
// * these options must match the settings on your gateway //
// * edit their values to match your configuration         //
// * frequency sub band is only relevant for the 915 bands //
// * either the network name and passphrase can be used or //
//     the network ID (8 bytes) and KEY (16 bytes)         //
/////////////////////////////////////////////////////////////
static std::string network_name = "MultiTech";
static std::string network_passphrase = "MultiTech";

static uint8_t network_id[] = APP_EUI;
static uint8_t network_key[] = APP_KEY;

static uint8_t frequency_sub_band = LORA_SUB_BAND;
static lora::NetworkType network_type = lora::PUBLIC_LORAWAN;
static uint8_t join_delay = 5;
static uint8_t ack = 1;
static bool adr = true;

mDot *dot = NULL;
lora::ChannelPlan *plan = NULL;

Serial pc(USBTX, USBRX);

// [START] Luminary global
LedHandler ledStatus(PA_4, false); // led indicador de estado
LedHandler ledLora(PA_5, false);   // led Indicador de alimentacion

// Declaracion componentes
CurrentSensor currentSensor(PB_12);
PhotoCell photoCell(PB_13, 0.65, 0.7);
DimmingCurves dimmingCurves;
DimmingDemo dimmingDemo;
LightController lightController(&photoCell, &dimmingCurves, &dimmingDemo, LightController::OpMode::Manual);
LightOutput lightOutput(PB_2, PB_0);

// otras variables
uint16_t loopDelay = 30; // amount of seconds between loops
bool bypassLoopDelay = false;

Timer lastMesureTimer; // timer para medicion de energia
LowPowerTimer lastClockSyncTimer;

// banderas para uplink
bool pendingSyncClock = false;
bool pendingSendTimestamp = false;
bool pendingSendConfig = false;

// [END] Luminary global

// [START] Luminary global

void payloadParser(uint8_t *RxBuffer, uint8_t RxBufferSize)
{
    logTrace("payloadParser");

    uint8_t saveBuffer[1] = {0};

    // [START] Parseo de payload

    switch (RxBuffer[0])
    {
    case 'A': // setear hora manualmente
    {
        uint32_t aux32 = 0;
        aux32 = static_cast<uint32_t>(RxBuffer[1] << 24);
        aux32 |= static_cast<uint32_t>(RxBuffer[2] << 16);
        aux32 |= static_cast<uint32_t>(RxBuffer[3] << 8);
        aux32 |= static_cast<uint32_t>(RxBuffer[4]);
        setManualTime(aux32);
        break;
    }

    case 'C': // Cambio de curva de dimming
    {
        logInfo("Switching to Curve %u", RxBuffer[1]);
        dimmingCurves.selectCurve(RxBuffer[1]);
        lightController.setOpMode(LightController::OpMode::AutoCurve);

        //save config
        saveBuffer[0] = static_cast<uint8_t>(LightController::OpMode::AutoCurve);
        dot->nvmWrite(DIR_OP_MODE, saveBuffer, 1);
        saveBuffer[0] = dimmingCurves.getCurrentCurve();
        dot->nvmWrite(DIR_CURVE, saveBuffer, 1);
        break;
    }

    case 'D': // Set manual dimming
    {
        logInfo("Switching to Manual %u%", RxBuffer[1]);
        lightController.setManualDimming(static_cast<float>(RxBuffer[1]) / 100);
        lightController.setOpMode(LightController::OpMode::Manual);

        //save config
        saveBuffer[0] = static_cast<uint8_t>(LightController::OpMode::Manual);
        dot->nvmWrite(DIR_OP_MODE, saveBuffer, 1);
        saveBuffer[0] = static_cast<uint8_t>(lightController.getManualDimLevel() * 100);
        dot->nvmWrite(DIR_MANUAL_DIMMING, saveBuffer, 1);
        break;
    }

    case 'E': // Enviar configuracion de light Contoller
    {
        pendingSendConfig = true;
        break;
    }

    case 'H': // enviar timestamp
    {
        pendingSendTimestamp = true;
        break;
    }

    case 'M': // Set mode
    {
        switch (RxBuffer[1])
        {
        case 0x00:
            logInfo("Switch to mode Manual");
            lightController.setOpMode(LightController::OpMode::Manual);
            saveBuffer[0] = static_cast<uint8_t>(LightController::OpMode::Manual);
            dot->nvmWrite(DIR_OP_MODE, saveBuffer, 1);
            break;
        case 0x01:
            logInfo("Switch to mode PhotoCell");
            lightController.setOpMode(LightController::OpMode::AutoPhotoCell);
            saveBuffer[0] = static_cast<uint8_t>(LightController::OpMode::AutoPhotoCell);
            dot->nvmWrite(DIR_OP_MODE, saveBuffer, 1);
            break;
        case 0x02:
            logInfo("Switch to mode dimming Curve");
            lightController.setOpMode(LightController::OpMode::AutoCurve);
            saveBuffer[0] = static_cast<uint8_t>(LightController::OpMode::AutoCurve);
            dot->nvmWrite(DIR_OP_MODE, saveBuffer, 1);
            break;
        case 0x03:
            logInfo("Switch to mode dimming Curve");
            lightController.setOpMode(LightController::OpMode::Demo);
            //! no se si conviene guardar el modo demo en memoria no volatil.
            saveBuffer[0] = static_cast<uint8_t>(LightController::OpMode::Demo);
            dot->nvmWrite(DIR_OP_MODE, saveBuffer, 1);
            break;

        default:
            logError("Mode not found");
            break;
        }
        break;
    }

    case 'R': // Software reset
    {
        NVIC_SystemReset();
        break;
    }

    case 'S': // Sincronizar hora
    {
        pendingSyncClock = true;
        break;
    }

    case 'T': // Cambiar loop delay
    {
        uint16_t aux16 = static_cast<uint16_t>(RxBuffer[1]) << 8;
        aux16 += RxBuffer[2];
        logInfo("Changing loop delay to %u seconds", aux16);
        loopDelay = aux16;
        dot->nvmWrite(DIR_LOOP_DELAY, &RxBuffer[1], 2);
        break;
    }
    }

    // bypass loop delay
    bypassLoopDelay = true;

    // [STOP] Parseo de payload
}

int main()
{
    // Custom event handler for automatically displaying RX data
    EventSlot events(&payloadParser);

    pc.baud(9600);

    mts::MTSLog::setLogLevel(mts::MTSLog::TRACE_LEVEL);

#if CHANNEL_PLAN == CP_US915
    plan = new lora::ChannelPlan_US915();
#elif CHANNEL_PLAN == CP_AU915
    plan = new lora::ChannelPlan_AU915();
#elif CHANNEL_PLAN == CP_EU868
    plan = new lora::ChannelPlan_EU868();
#elif CHANNEL_PLAN == CP_KR920
    plan = new lora::ChannelPlan_KR920();
#elif CHANNEL_PLAN == CP_AS923
    plan = new lora::ChannelPlan_AS923();
#elif CHANNEL_PLAN == CP_AS923_JAPAN
    plan = new lora::ChannelPlan_AS923_Japan();
#elif CHANNEL_PLAN == CP_IN865
    plan = new lora::ChannelPlan_IN865();
#endif
    assert(plan);

    dot = mDot::getInstance(plan);
    assert(dot);

    logInfo("mbed-os library version: %d.%d.%d", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
    logInfo("Promatix SmartCell version: %d.%d.%d", PROMATIX_VERSION_MAJOR, PROMATIX_VERSION_MINOR, PROMATIX_VERSION_PATCH);

    // start from a well-known state
    logInfo("defaulting Dot configuration");
    dot->resetConfig();
    dot->resetNetworkSession();

    // make sure library logging is turned on
    dot->setLogLevel(mts::MTSLog::DEBUG_LEVEL);

    // attach the custom events handler
    dot->setEvents(&events);

    // update configuration if necessary
    if (dot->getJoinMode() != mDot::OTA)
    {
        logInfo("changing network join mode to OTA");
        if (dot->setJoinMode(mDot::OTA) != mDot::MDOT_OK)
        {
            logError("failed to set network join mode to OTA");
        }
    }
    // in OTA and AUTO_OTA join modes, the credentials can be passed to the library as a name and passphrase or an ID and KEY
    // only one method or the other should be used!
    // network ID = crc64(network name)
    // network KEY = cmac(network passphrase)
    // update_ota_config_name_phrase(network_name, network_passphrase, frequency_sub_band, network_type, ack);
    update_ota_config_id_key(network_id, network_key, frequency_sub_band, network_type, ack);

    // configure the Dot for class C operation
    // the Dot must also be configured on the gateway for class C
    // use the lora-query application to do this on a Conduit: http://www.multitech.net/developer/software/lora/lora-network-server/
    // to provision your Dot for class C operation with a 3rd party gateway, see the gateway or network provider documentation
    logInfo("changing network mode to class C");
    if (dot->setClass("C") != mDot::MDOT_OK)
    {
        logError("failed to set network mode to class C");
    }

    // enable or disable Adaptive Data Rate
    dot->setAdr(adr);

    // Configure the join delay
    dot->setJoinDelay(join_delay);

    // save changes to configuration
    logInfo("saving configuration");
    if (!dot->saveConfig())
    {
        logError("failed to save configuration");
    }

    // display configuration
    display_config();

    // [START] init Luminary
    logInfo("========================");
    logInfo("LUMINARY version");
    logInfo("========================");

    // encender led lora hasta que haya join
    ledStatus.setCicle(LED_SEQUENCE_OK);
    ledLora.setCicle(LED_SEQUENCE_ERROR_1);

    // read config
    uint8_t saveBuffer[3] = {0, 0, 0};

    // leer version de firmware para la que se crearon los datos
    // y comparar con la version actual
    if (dot->nvmRead(DIR_PROMATIX_VERSION_MAJOR, saveBuffer, 3))
    {
        if ((saveBuffer[0] != PROMATIX_VERSION_MAJOR) || (saveBuffer[1] != PROMATIX_VERSION_MINOR) || (saveBuffer[2] != PROMATIX_VERSION_PATCH))
        {
            // la version no coincide, entonces cargar los datos por defecto
            logInfo("Loading default values to memory");

            saveBuffer[0] = PROMATIX_VERSION_MAJOR;
            saveBuffer[1] = PROMATIX_VERSION_MINOR;
            saveBuffer[2] = PROMATIX_VERSION_PATCH;
            dot->nvmWrite(DIR_PROMATIX_VERSION_MAJOR, saveBuffer, 3);

            saveBuffer[0] = 0x02; // loop delay cada 10 minutos
            saveBuffer[1] = 0x58;
            dot->nvmWrite(DIR_LOOP_DELAY, saveBuffer, 2);

            saveBuffer[0] = 0x01; // Modo de operacion Fotocelda
            dot->nvmWrite(DIR_OP_MODE, saveBuffer, 1);

            saveBuffer[0] = 0x00; // Curva seleccionada 1
            dot->nvmWrite(DIR_CURVE, saveBuffer, 1);

            saveBuffer[0] = 100; // Dimming manual 100%
            dot->nvmWrite(DIR_MANUAL_DIMMING, saveBuffer, 2);
        }
    }

    // leer loop delay
    if (dot->nvmRead(DIR_LOOP_DELAY, saveBuffer, 2))
    {
        loopDelay = static_cast<uint16_t>(saveBuffer[0]) << 8;
        loopDelay += saveBuffer[1];
    }
    else
        logError("Failed to read saved loop delay");

    // leer nivel de dimming manual
    if (dot->nvmRead(DIR_MANUAL_DIMMING, saveBuffer, 1))
        lightController.setManualDimming(static_cast<float>(saveBuffer[0]) / 100);
    else
        logError("Failed to read saved manual dimming level");

    // leer opMode
    if (dot->nvmRead(DIR_OP_MODE, saveBuffer, 1))
    {
        // evitar configurar para curva antes de obtener la hora
        if (static_cast<LightController::OpMode>(saveBuffer[0]) == LightController::OpMode::AutoCurve)
            lightController.setOpMode(LightController::OpMode::AutoPhotoCell);
        else
            lightController.setOpMode(static_cast<LightController::OpMode>(saveBuffer[0]));
    }
    else
        logError("Failed to read saved operation mode");

    // leer current curve
    if (dot->nvmRead(DIR_CURVE, saveBuffer, 1))
        dimmingCurves.selectCurve(saveBuffer[0]);
    else
        logError("Failed to read saved curve index");

    // Initial delay
    srand(photoCell.read(1));               // Iniciamos la semilla de numeros aleatorios leyendo la luz
    wait_us((rand() % (100 + 1)) * 100000); // esperamos un tiempo aleatorio (entre 0 y 10s) antes de mandar el join

#if ENABLE_JOIN == 1
        // Intentamos Join y si es exitoso
    join_network(24);

#endif

    // iniciar timer de medicion de energia
    lastMesureTimer.start();

    // iniciar timer de sincronizacion de reloj
    lastClockSyncTimer.start();

    // [END] init Luminary

    while (true)
    {
        // [START] Luminary Loop

#if ENABLE_JOIN == 1
        // Intentamos Join.
        if (!dot->getNetworkJoinStatus())
        {
            join_network(8);
        }

#endif
        // Show time
        printTime();

#if ENABLE_JOIN == 1
        // Sincronizar hora si no esta sincronizado o cada 12 horas.
        // if ( pendingSyncClock || !timeIsSynced() || lastClockSyncTimer.read() > 43200)
        if (pendingSyncClock || !timeIsSynced() || lastClockSyncTimer.read() > 43200)
        {
            pendingSyncClock = false;
            logInfo("Attemting to sync clock");
            if (syncTime(5, TIME_ZONE))
            {
                lastClockSyncTimer.reset();
            }
        }
#endif

        // enviar timestamp si estaba pendiente
        if (pendingSendTimestamp)
        {
            pendingSendTimestamp = false;
            send_currentTime();
        }

        // si esta en hora, recuperar modo curvas si esta guardado
        if (timeIsSynced())
        {
            if (dot->nvmRead(DIR_OP_MODE, saveBuffer, 1))
            {
                lightController.setOpMode(static_cast<LightController::OpMode>(saveBuffer[0]));
            }
        }

        // enviar configuracion si estaba pendiente
        if (pendingSendConfig)
        {
            pendingSendConfig = false;
            send_smartCellConfig(lightController.getMode(), static_cast<uint8_t>(lightController.getManualDimLevel() * 100), loopDelay);
        }

        // Energy calculation
        float power = currentSensor.getCurrent() * 220;
        float timeSinceLastMesure = lastMesureTimer.read();
        lastMesureTimer.reset();
        float energy = (power * timeSinceLastMesure) / 3600;

        // Cambiar el nivel de dimming segun la hora
        float dimming = lightController.getDimming(getHour());
        lightOutput.setOutput(dimming);

        // medicion de potencia
        wait_us(500000); // retardo de 500ms para que se estabilice la corriente antes de medirla
        power = currentSensor.getCurrent() * 220;

        // print config
        logInfo("========================");
        logInfo("SmartCell configuration");
        logInfo("========================");
        logInfo("Has Joined ====== %s", dot->getNetworkJoinStatus() ? "true" : "false");
        lightController.printMode();
        logInfo("Dimming ========= %.0f %", dimming * 100);
        logInfo("Power =========== %.2fW", power);
        logInfo("Energy ========== %.2fkW", energy);
        logInfo("Last mesure ===== %.0f seconds ago", timeSinceLastMesure);
        logInfo("Period ========== %us", loopDelay);

        // check if error
        if (dimming > 0)
        {
            if (power > 0)
                ledStatus.setCicle(LED_SEQUENCE_OK);
            else
                ledStatus.setCicle(LED_SEQUENCE_ERROR_1);
        }
        else if (power > 0)
            ledStatus.setCicle(LED_SEQUENCE_ERROR_2);

        // Send Light Status
        if (dot->getNetworkJoinStatus())
        {
            if (send_lightStatus(dimming, power, energy))
            {
                ledLora.setCicle(LED_SEQUENCE_OK);
            }
            else
            {
                ledLora.setCicle(LED_SEQUENCE_ERROR_2);
            }
        }
        else
        {
            ledLora.setCicle(LED_SEQUENCE_ERROR_1);
        }

        // [END] Luminary Loop

        // the Dot can't sleep in class C mode
        // it must be waiting for data from the gateway

        logInfo("waiting for %us", loopDelay);
        for (uint16_t i = 0; i < loopDelay; i++)
        {
            // Saltarse el retardo si se activa el bypass
            if (bypassLoopDelay)
            {
                logInfo("Bypassing loop delay");
                bypassLoopDelay = false;
                break;
            }

            // Reducir el retardo si esta en modo demostracion
            if (lightController.getMode() == static_cast<uint8_t>(LightController::OpMode::Demo))
            {
                if (i >= dimmingDemo.demoPeriod)
                {
                    break;
                }
            }
            wait_us(1000000);
        }
    }

    return 0;
}

#endif
