load('api_config.js');
load('api_gpio.js');
load('api_mqtt.js');
load('api_dht.js');
load('api_net.js');
load('api_uart.js');
load('api_sys.js');
load('api_timer.js');

let uartNo = 0; // 1 for only debugging, no input

// SFMD Parameters
let arr2 = [];
let sendEmail = 0;
let sts = 0;
let stsPrev = 0;
let irms = 0;
let v1 = 0;
let v2 = 0;
let stsMCB = 'Power OK';

let test = '\x00\x01\x02\x03\x04\x05';
let rxAcc = '';   // Accumulated Rx data, will be echoed back to Tx
let value = false;

// Configuration for devices attached to the thing
let led = 16; // 2; // Cfg.get('app.led'); //pin2

// /* UART Config */
UART.setConfig(uartNo,{
 baudRate: 9600,
 rxBufSize: 1024,
 txBufSize: 1024,
});

// Getting name of the thing from config
let thingName = Cfg.get('aws.thing_name');

// Setting pin mode for led as output
GPIO.set_mode(led, GPIO.MODE_OUTPUT);

// All AWS thing shadow topics
let updateTopic = "$aws/things/"+thingName+"/shadow/update";
let updateAcceptedTopic = "$aws/things/"+thingName+"/shadow/update/accepted";
let updateRejectedTopic = "$aws/things/"+thingName+"/shadow/update/rejected";
let updateDeltaTopic = "$aws/things/"+thingName+"/shadow/update/delta";

// function to publish data to thing shadow
function report(){
  print ("updating ", (GPIO.read(led)));
  let topic = updateTopic;
    // TODO
    let message = JSON.stringify({
      "state":{
       "reported":{
          "device59.195": GPIO.read(led)===0?1:0,  //for LED (test only)
          "device59.203": sendEmail, // GPIO.read(led)===0?1:0,
          "device59.192": irms, // RMS Current mili ampere
          "device59.193": v1, // RMS Primary Voltage
          "device59.194": v2, // RMS Secondary Voltage
          "device59.191": sts,  // Status flag
          "device59.202": stsMCB //MCB Status string, sent as first parameter of JSON
       }
     }
  });
  
 
 // get(state.reported, 'device59.202') AS MCBStatus, get(state.reported, 'device59.192') AS Current(mA), 
 // get(state.reported, 'device59.193') AS VoltagePrimary, get(state.reported, 'device59.194') AS VoltageSecondary
 
  let res = MQTT.pub(updateTopic, message, 1);
  //if(res){
  //  GPIO.write(led,1);
  //} else {
  //  GPIO.write(led,0);
  //}
  //print('Published: ', res ? 'yes' : 'no', 'Response',res);
	  
}

// calls report method after 5000 seconds
Timer.set(5000 /* 5000 milisec */, true /* repeat */, function() {
//Timer.set(Cfg.get('app.interval'), true, function() {
	bu(); //user function to be called every 5 sec
}, null);

// Parse String from UART data
function readString() {
	//let test = '\x00\x27\x10\xF5\xF0\x55\x66\x77\xa0\x11\x06bx\xfe\xed\xa3Z\xaa\xa5\x05}\xf7\x04Yj\xe2f\x0b\xe7';
		print('=== test:', test);
		stsPrev = sts;	
		sts = test.at(0);
		irms = (test.at(1))*256 + (test.at(2));
		v1 = test.at(3);
		v2 = test.at(4);
		if (sts === 0) {
		   stsMCB = 'Power OK';
		} else if (sts === 1) {
		   stsMCB = 'MCB Tripped';
		} else { // if (sts === 2) {
		   stsMCB = 'No Power';
		}	
        sendEmail = (sts !== stsPrev ? 1 : 0);  // set email flag only on state change
		
		print('=== F,I,V1,V2 ',sts, irms, v1,v2,stsMCB, sendEmail);	//debug msg
}		

/*
Funtion bu: reads 5 byte string, parses it, sends report
When the available buffer at UART 0 is greater 5bytes,
we will send this data to the MQTT topic. In additon we also turn off the blue
led (default one) whenever the data is available and being sent.
*/

function bu(){
  UART.setDispatcher(uartNo, function(uartNo) {	
     let ra = UART.readAvail(uartNo);
    if ( ra > 5 ) { // read buffer > 5 bytes
      test = UART.read(uartNo); //read 5 bytes
      print("=== UART data:", test);
	  readString(); // parse UART data 
	  report(); //publish report
	  GPIO.toggle(led); // Toggle LED on each UART reception
      UART.flush(uartNo); // flush buffer for next reception
    }
  }, null);
}

UART.setRxEnabled(uartNo, true);

// Subscription to AWS thing shadow topics
MQTT.sub(updateAcceptedTopic, function(conn, topic, msg) {
  print('Topic:', topic, 'message:', JSON.stringify(msg));
}, null);

MQTT.sub(updateRejectedTopic, function(conn, topic, msg) {
  print('Topic:', topic, 'message:', JSON.stringify(msg));
}, null);

MQTT.sub(updateDeltaTopic, function(conn, topic, msg) {
  print('Topic:', topic, 'message:', JSON.stringify(msg));
  let m = JSON.parse(msg);
  // TODO
  GPIO.write(led, m.state["device59.195"]===0?1:0); //abhi
  //GPIO.write(led, m.state["device59.180"]===0?1:0); //abhi
  report();
}, null);

// Monitor network connectivity.
Net.setStatusEventHandler(function(ev, arg) {
  let evs = '???';
  if (ev === Net.STATUS_DISCONNECTED) {
    evs = 'DISCONNECTED';
  } else if (ev === Net.STATUS_CONNECTING) {
    evs = 'CONNECTING';
  } else if (ev === Net.STATUS_CONNECTED) {
    evs = 'CONNECTED';
  } else if (ev === Net.STATUS_GOT_IP) {
    evs = 'GOT_IP';
  }
 // print('== Net event:', ev, evs); // for debug
}, null);
