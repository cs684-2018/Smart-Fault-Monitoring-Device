load('api_config.js');
load('api_gpio.js');
load('api_mqtt.js');
load('api_dht.js');
load('api_net.js');
load('api_uart.js');
load('api_sys.js');
load('api_timer.js');

let uartNo = 0; // 1 for only debugging, no input
/*
let LED_OFF = 0;
let LED_ON = 1;
let relay = 5; // D1 on NodeMCU
let reset = 4; // D2 on NodeMCU
let isConnected = false;
let isMQTTConnected = false;
*/

// SFMD Parameters
let arr2 = [];
let sendEmail = 0;
let sts = 0;
let stsPrev = 0;
let irms = 0;
let v1 = 0;
let v2 = 0;
let stsMCB = 'Power OK';

//let data;
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
          "device59.195": GPIO.read(led)===0?1:0,
          //"device59.180": GPIO.read(led)===False?True:False,//for led           
          "device59.203": sendEmail, // GPIO.read(led)===0?1:0,
          "device59.192": irms, 
          "device59.193": v1, 
          "device59.194": v2, 
          "device59.191": sts, 
          "device59.202": stsMCB // sent as first parameter of JSON
       }
     }
  });
  //MQTT.pub(updateTopic, message, 1);
  let res = MQTT.pub(updateTopic, message, 1);
  //if(res){
  //  GPIO.write(led,1);
  //} else {
  //  GPIO.write(led,0);
  //}
  //print('Published: ', res ? 'yes' : 'no', 'Response',res);
	  
}

// calls report method after 5000 seconds
//Timer.set(1000 /* milliseconds */, Timer.REPEAT, function() {
Timer.set(5000 /* 2 sec */, true /* repeat */, function() {
//Timer.set(Cfg.get('app.interval'), true, function() {
	// value = !value;
	// GPIO.write(led,value);
    //test = JSON.stringify((rxAcc.length > 5 ? rxAcc : ''));
    //test = JSON.stringify(rxAcc);
    //test = (rxAcc.length > 5 ? (' Rx: ' + rxAcc) : '');
    //GPIO.write(led, GPIO.read(led)===0?1:0);  //Abhi
	//// let value = GPIO.toggle(led);
	bu();
	//readString(); // read parameters
    //report();
	// rxAcc = '';
	
}, null);

// Parse String
function readString() {
	//let test = '\x00\x27\x10\xF5\xF0\x55\x66\x77\xa0\x11\x06bx\xfe\xed\xa3Z\xaa\xa5\x05}\xf7\x04Yj\xe2f\x0b\xe7';
	//	test = data;
		/*
		print('=== test:', test);
		print('===      JSON.stringify(test)', JSON.stringify(test));
		for (let i = 0; i < test.length; i++) {
			arr2[i] = test.at(i);
		}
		*/
		//print('===      arr2', arr2);
		//print('===      JSON.stringify(arr2)', JSON.stringify(arr2));
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
        sendEmail = (sts !== stsPrev ? 1 : 0);
		
		print('=== F,I,V1,V2 ',sts, irms, v1,v2,stsMCB, sendEmail);	
}		

/*
When the available buffer at UART 0 is greater 5bytes,
we will send this data to the MQTT topic. In additon we also turn off the blue
led (default one) whenever the data is available and being sent.
*/

function bu(){
  UART.setDispatcher(uartNo, function(uartNo) {	
 // UART.setDispatcher(uartNo, function(uartNo, ud) {
    // print("UART dispacther - uart:",uartNo);
	//GPIO.toggle(led);

    let ra = UART.readAvail(uartNo);
   // let oa = UART.writeAvail(uartNo);
    if ( ra > 5 ) { // read buffer > 200 bytes
      test = UART.read(uartNo);
      //let data = UART.read(uartNo);
      print("=== UART data:", test);
	  readString(); //
	  report();
	//UART.flush(uartNo); // test ?
	//value = !value;
	//GPIO.write(led,value);
	//GPIO.write(led,!GPIO.read(led));
	  GPIO.toggle(led); ///// 
    
     //  let res = MQTT.pub(topic, JSON.stringify({ "data": data }), 1);
      // if(res){
        // GPIO.write(led,1);
      // } else {
        // GPIO.write(led,0);
      // }
      // print('Published: ', res ? 'yes' : 'no', 'Response',res);
	  // 
	  UART.flush(uartNo); // test ?
    }
  }, null);

//UART.setRxEnabled(uartNo, true);
}

UART.setRxEnabled(uartNo, true);
//*/


/*
// Set dispatcher callback, it will be called whenver new Rx data or space in
// the Tx buffer becomes available
UART.setDispatcher(uartNo, function(uartNo) {
  let ra = UART.readAvail(uartNo);
  //if (ra > 0) {
  if (ra > 5) {
    // Received new data: print it immediately to the console, and also
    // accumulate in the "rxAcc" variable which will be echoed back to UART later
    //let data = UART.read(uartNo);
    test = UART.read(uartNo);
    print('Received UART data:', test);
    //rxAcc += data;
    //rxAcc = data;
	//UART.flush(uartNo); // test ?
	value = !value;
	GPIO.write(led,value);
  }
}, null);

UART.setRxEnabled(uartNo, true);
*/
///////

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
 // print('== Net event:', ev, evs); //ABHI
}, null);
