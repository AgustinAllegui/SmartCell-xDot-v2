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

// [STOP] Luminary Includes

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
DigitalOut led1(PA_4);
DigitalOut led2(PA_5);

// Declaracion componentes
CurrentSensor currentSensor(PB_12);
PhotoCell photoCell(PB_13, 0.65, 0.7);
DimmingCurves dimmingCurves;
LightController(&photoCell, &dimmingCurves, LightController::Manual);
LightOutput lightOutput(PB_2, PB_0);

// otras variables
unsigned int loopDelay = 10; // amount of seconds between loops
bool bypassLoopDelay = false;

// [END] Luminary global

void payloadParser(uint8_t *RxBuffer, uint8_t RxBufferSize)
{
    logTrace("payloadParser");

    uint8_t saveBuffer[1] = {0};

    // [START] Parseo de payload

    switch (RxBuffer[0])
    {
    case 'C': // Cambio de curva de dimming
        logInfo("Switching to Curve %u", RxBuffer[1]);
        dimmingCurves.selectCurve(RxBuffer[1]);
        lightController.setOpMode(LightController::AutoCurve);

        //save config
        saveBuffer[1] = LightController::AutoCurve;
        dot->nvmWrite(DIR_OP_MODE, saveBuffer, 1);
        saveBuffer[1] = dimmingCurves.getCurrentCurve();
        dot->nvmWrite(DIR_CURVE, saveBuffer, 1);
        break;

    case 'D': // Set manual dimming
        logInfo("Switching to Manual %u%", RxBuffer[1]);
        lightController.setManualDimming(static_cast<float>(RxBuffer[1]) / 100);
        lightController.setOpMode(LightController::Manual);

        //save config
        saveBuffer[1] = LightController::Manual;
        dot->nvmWrite(DIR_OP_MODE, saveBuffer, 1);
        saveBuffer[1] = dimmingCurves.getCurrentCurve();
        dot->nvmWrite(DIR_MANUAL_DIMMING, saveBuffer, 1);
        break;

    case 'M': // Set mode
        switch (RxBuffer[1])
        {
        case 0x00:
            logInfo("Switch to mode Manual");
            lightController.setOpMode(LightController::Manual);
            saveBuffer[1] = LightController::Manual;
            dot->nvmWrite(DIR_OP_MODE, saveBuffer, 1);
            break;
        case 0x01:
            logInfo("Switch to mode PhotoCell");
            LightController.setOpMode(LightController::AutoPhotoCell);
            saveBuffer[1] = LightController::AutoPhotoCell;
            dot->nvmWrite(DIR_OP_MODE, saveBuffer, 1);
            break;
        case 0x02:
            logInfo("Switch to mode dimming Curve");
            lightController.setOpMode(LightController::AutoCurve);
            saveBuffer[1] = LightController::AutoCurve;
            dot->nvmWrite(DIR_OP_MODE, saveBuffer, 1);
            break;
        default:
            logError("Mode not found");
            break;
        }
        break;

    default:
        logError("Command not found");
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

    // start from a well-known state
    logInfo("defaulting Dot configuration");
    dot->resetConfig();
    dot->resetNetworkSession();

    // make sure library logging is turned on
    dot->setLogLevel(mts::MTSLog::INFO_LEVEL);

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

    // read config
#if ENABLE_READ_NON_VOLATILE == 1

#endif

    // Initial delay
    srand(photoCell.read(1));        // Iniciamos la semilla de numeros aleatorios leyendo la luz
    wait_us((rand() % 21) * 100000); // esperamos un tiempo aleatorio (entre 0 y 2s) antes de mandar el join

#if ENABLE_JOIN == 1
    // Intentamos Join y si es exitoso, sincronizar la hora.
    join_network(24);
    if (dot->getNetworkJoinStatus())
    {
        logInfo("Attemting to sync clock");
        set_time((dot->getGPSTime() / 1000) + 315964800 + (TIME_ZONE * 3600));
    }
#endif

    // [END] init Luminary

    while (true)
    {
        std::vector<uint8_t> tx_data;

#if ENABLE_JOIN == 1
        // Intentamos Join y si es exitoso, sincronizar la hora.
        if (!dot->getNetworkJoinStatus())
        {
            join_network(8);
            if (dot->getNetworkJoinStatus())
            {
                logInfo("Attemting to sync clock");
                set_time((dot->getGPSTime() / 1000) + 315964800 + (TIME_ZONE * 3600));
            }
        }
#endif
        // Show time
        time_t currentTimestamp = time(NULL);
        logInfo("Current Time: %s", ctime(&currentTimestamp));

        // [START] Luminary Loop

        // blink led
        led1 = (led1) ? 0 : 1;

        // set output
        struct tm *timeStruct;
        timeStruct = gmtime(currentTimestamp);

        float dimming = lightController.getDimming(timeStruct->tm_hour);
        lightOutput.setOutput(dimming);

        float power = currentSensor.getCurrent() * 220;

        // print config
        logInfo("========================");
        logInfo("SmartCell configuration");
        logInfo("========================");
        logInfo("Is Joined ======= %s", dot->getNetworkJoinStatus() ? "true" : "false");
        lightController.printMode();
        logInfo("Dimming ========= %.0f%", dimming * 100);
        logInfo("Power =========== %.2fW", current);
        logInfo("Period ========== %us", loopDelay);

        // prepare payload
        if (dot->getNetworkJoinStatus())
        {
            tx_data.clear();
            tx_data.push_back(reinterpret_cast<uint8_t>('S'));

            uint16_t aux16 = static_cast<uint16_t>(power);
            tx_data.push_back(static_cast<uint8_t>(power >> 8));
            tx_data.push_back(static_cast<uint8_t>(power));

            tx_data.push_back(static_cast<uint8_t>(dimming * 100));
        }

        // [END] Luminary Loop

        if (dot->getNetworkJoinStatus())
        {
            send_data(tx_data);
        }

        // the Dot can't sleep in class C mode
        // it must be waiting for data from the gateway

        logInfo("waiting for %us", loopDelay);
        for (unsigned int i = 0; i < loopDelay; i++)
        {
            if (bypassLoopDelay)
            {
                logInfo("Bypassing loop delay");
                bypassLoopDelay = false;
                break;
            }
            wait_us(1000000);
        }
    }

    return 0;
}

#endif
