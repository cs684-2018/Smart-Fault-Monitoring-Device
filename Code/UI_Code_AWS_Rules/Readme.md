e-Yantra AWS Platform used for front-end Dashboard. 
Procedure to compile ESP code & configuring thing/device on AWS e-yantra 
is mentioned in 'how-to.txt' file.

Following rules were set for triggering the email.

AWS SNS Rule:
-------------

Data:
get(state.reported, 'device59.202') AS MCBStatus, get(state.reported, 'device59.192') AS Current_mA, get(state.reported, 'device59.193') AS VoltagePrimary, get(state.reported, 'device59.194') AS VoltageSecondary


Condition for email trigger:
get(state.reported, 'device59.203') = 1


SNS Settings:

SNS Topic	smart_fault_monitoring
Topic ARN	arn:aws:sns:ap-southeast-1:362702149756:smart_fault_monitoring

SNS Subscriptions:
Protocol	Value
email		abhishek.pal.iot@gmail.com
email		manjrekarom@gmail.com
