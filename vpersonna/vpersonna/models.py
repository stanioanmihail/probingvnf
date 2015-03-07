class User():
	client_id = Integer()
	client_name = String()
	phone_number = String()
	address = String()
	card_id = String()
	contract_id = String()
	contract_type = String()

class Device():
	device_id = Integer()
	device_type = String()
	client_id = Integer() #fk
	
class Remote():
	ip_address = Integer()
	port_number = Integer()
	protocol_type = String()
	app_protocol_type = String()

class TrafficSession():
	device_id = Integer() #fk
	remote_id = Integer() #fk
	start_time = Time()
	end_time = Time()
	packet_counter = Integer()
	traffic size = Integer()
	packets_droped = Integer()
	meta = String()
	
