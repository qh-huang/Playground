#include "msgbus.h"

struct WeatherInfo
{
    int temperature;
    int humidity;
};

struct StockInfo
{
    uint16_t stock_id;
    double price;
};

// for demo purpose only. start from 0x F000 0000 to avoid confliction against the library
namespace DataType {
    constexpr DataId WEATHER_INFO = 0x0F0000001;
    constexpr DataId STOCK_INFO = 0x0F0000002;
}

DEFINE_MSG(WeatherInfo, WEATHER_INFO)
DEFINE_MSG(StockInfo, STOCK_INFO)

class WeatherMan : public MsgBus::Subscriber
{
public:
    WeatherMan(): MsgBus::Subscriber("weatherman") {}
    bool ProcMsg(MsgPtr msg) {
        if (msg->data_id == DataType::WEATHER_INFO) {
            shared_ptr<WeatherInfoMsg> wmp = dynamic_pointer_cast<WeatherInfoMsg>(msg);
            cout << "temperature: " << wmp->data.temperature << endl;
            cout << "humidity: " << wmp->data.humidity << endl;
        } else {
            cout << "WeatherMan ignore msg: " << msg->msg_name; 
        }
        return true;
    }
};

class StockTrader : public MsgBus::Subscriber
{
public:
    StockTrader(): MsgBus::Subscriber("stock_trader") {}

    // if additional tasks need to be done before activation,
    // override Activate (or just do it in constructor)
    void Activate() {
        Subscribe(DataType::STOCK_INFO);
        Subscriber::Activate();
    }

    bool ProcMsg(MsgPtr msg) {
        if (msg->data_id == DataType::STOCK_INFO) {
            StockInfoMsgPtr sp = dynamic_pointer_cast<StockInfoMsg>(msg);
            cout << "stock id: " << sp->data.stock_id << endl;
            cout << "stock price: " << sp->data.price << endl;
        } else {
            cout << "StockTrader ignore msg: " << msg->msg_name; 
        }
        return true;
    }        
    
};

class WeatherInfoBroadcaster : public Looper, public MsgBus::Publisher 
{
public:
    WeatherInfoBroadcaster() : Looper("weather_broadcase", 500) {}

    void SpinOnce() override {
        static int temperature = 20;
        static int humidity = 1;
        WeatherInfoMsgPtr wp = make_shared<WeatherInfoMsg>();
        wp->data.temperature = temperature++;
        wp->data.humidity = humidity++;
        Publish(wp);
    }

    bool WaitCondition() override {
        return true;
    }
};

class StockInfoBroadcaster : public Looper, public MsgBus::Publisher
{
public:
    StockInfoBroadcaster() : Looper("stock_brdcst", 1000) {}

    void SpinOnce() override {
        static int stock_id = 1;
        static double price = 10.3;
        StockInfoMsgPtr sp = make_shared<StockInfoMsg>();
        sp->data.stock_id = stock_id;
        sp->data.price = price;
        price += 0.7;
        Publish(sp);
    }

    bool WaitCondition() override {
        return true;
    }
};

int main(int argc, char* argv[])
{
    WeatherInfo wi;
    wi.temperature = 23;
    wi.humidity = 11;

    StockInfo si;
    si.stock_id = 123;
    si.price = 12345.6;

    WeatherInfoMsgPtr wp = make_shared<WeatherInfoMsg>(wi);
    StockInfoMsgPtr si_ptr = make_shared<StockInfoMsg>(si);

    shared_ptr<WeatherMan> wm = make_shared<WeatherMan>();
    wm->Subscribe(DataType::WEATHER_INFO); // subsciption done here in caller's context
    wm->Activate(); // this is Looper::Activate(), not overrided by WeatherMan

    shared_ptr<StockTrader> stock_trader = make_shared<StockTrader>();
    stock_trader->Activate(); // subscription done in StockTrader::Activate()

    // this_thread::sleep_for(std::chrono::milliseconds(2000));

    // example of publishing by calling MsgBus::Publish
    MsgBus::Publish(wp);
    MsgBus::Publish(si_ptr);

    // example of publishing by MsgBus::Publisher
    WeatherInfoBroadcaster wb;
    StockInfoBroadcaster sb;
    wb.Activate();
    sb.Activate();

    int i;
    cin >> i;

    return 0;
}