import smbus
import time
import os
import temperature #temperature stub
import plotly.plotly as plot #plotly API
import json #extract credentials
import datetime #temp-time chart association

# for RPI version 1, use "bus = smbus.SMBus(0)"
bus = smbus.SMBus(1)

# This is the address we setup in the Arduino Program
address = 0x04

temperature = 0

def init_plotly_stream():
    # init credentials
    with open('./credentials.json') as credentials_file:
        plotly_user_config = json.load(credentials_file)

    # authenticate with provided credentials
    plot.sign_in(plotly_user_config["plotly_username"], plotly_user_config["plotly_api_key"])

    # configure the plot
    url = plot.plot([
        {
            'x': [],
            'y': [],
            'type': 'scatter',
            'mode':'lines+markers',
            'stream': {
                'token': plotly_user_config['plotly_streaming_tokens'][0],
                'maxpoints': 200
                }
        }], filename='MS-IOT Temp')

    print ("View your streaming graph here: ", url)

    # attach a stream to the plot
    stream = plot.Stream(plotly_user_config['plotly_streaming_tokens'][0])

    return stream

def read_and_post(stream):
    # read temperature from our stub module
    temp = temperature
    # post data to plotly
    stream.write({'x': datetime.datetime.now(), 'y': temp})

def writeNumber(value):
        bus.write_byte(address, value)
	# bus.write_byte_data(address, 0, value)
        return -1

def readNumber():
        number = bus.read_byte(address)
	# number = bus.read_byte_data(address, 1)
        return number


stream = init_plotly_stream()
stream.open()

while True:
        os.system('clear')
        while(readNumber()!=255):
                pass

        temperature = readNumber()

        print ("Current Temperature ", temperature, "C")
	
        number = readNumber()
        print("Sensor distance: ", number,"mm" )
        number = readNumber()*100
        print("Motor Position: ", number)
        read_and_post(stream)
        time.sleep(2)
