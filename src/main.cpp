#include <mbed.h>
#include <EthernetInterface.h>
#include <rtos.h>
#include <mbed_events.h>

Thread eventhandler;
EventQueue eventqueue;

/* YOU will have to hardwire the IP address in here */
SocketAddress server("192.168.4.214",65500);
EthernetInterface eth;
UDPSocket udp;
char buffer[256];

void sendstate(const char *button, const char *state){
    char buffer[56];
    sprintf(buffer,"%s:%s\n\0",button,state);
    printf("Sending : %s ",buffer);
    nsapi_size_or_error_t r = udp.sendto( server, buffer, strlen(buffer));
    printf("(%d bytes)\n",r);
}

InterruptIn sw2(SW2);
InterruptIn sw3(SW3);
void press(const char *b){
    sendstate(b,"pressed");
}
void release(const char *b){
    sendstate(b,"released");
}

/* function  to poll the 5 way joystich switch */
enum { Btn1, Btn2, sw_up, sw_down, sw_left, sw_right, sw_center};
const char *swname[] = {"SW2","SW3","Up","Down","Left","Right","Center"};
struct pushbutton {
    DigitalIn sw;
    bool invert;
} buttons[] = {
  {DigitalIn(SW2),true},
  {DigitalIn(SW3),true},
  {DigitalIn(A2),false},
  {DigitalIn(A3),false},
  {DigitalIn(A4),false},
  {DigitalIn(A5),false},
  {DigitalIn(D4),false},
};

bool ispressed(int b) {
  return (buttons[b].sw.read())^buttons[b].invert;
}
void jspoll(void) {
    int b;
    for(b=sw_up ; b<=sw_center ; b++) {
        if( ispressed(b) ){
            sendstate(swname[b],"pressed");
        }else{
            sendstate(swname[b],"released");
        }
    }
}

int main() {
    printf("conecting \n");
    eth.connect();
    const char *ip = eth.get_ip_address();
    printf("IP address is: %s\n", ip ? ip : "No IP");

    udp.open( &eth);
    SocketAddress source;
        printf("sending messages to %s/%d\n",
                server.get_ip_address(),  server.get_port() );

    eventhandler.start(callback(&eventqueue, &EventQueue::dispatch_forever));

    sw2.fall(eventqueue.event(press,"SW2"));
    sw3.fall(eventqueue.event(press,"SW3"));

    sw2.rise(eventqueue.event(release,"SW2"));
    sw3.rise(eventqueue.event(release,"SW3"));

    eventqueue.call_every(500,jspoll);

    while(1){}
}
