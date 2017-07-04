import time
import smbus2
import bme280
import requests
import argparse

# constants
SENSOR_PORT = 1
SENSOR_ADDRESS = 0x76

URL = 'https://corlysis.com:8086/write'
READING_DATA_PERIOD_MS = 5000.0
SENDING_PERIOD = 2
MAX_LINES_HISTORY = 1000


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("db", help="database name")
    parser.add_argument("token", help="secret token")
    args = parser.parse_args()

    corlysis_params = {"db": args.db, "u": "token", "p": args.token, "precision": "ms"}

    # initialization
    bus = smbus2.SMBus(SENSOR_PORT)
    bme280.load_calibration_params(bus, SENSOR_ADDRESS)

    payload = ""
    counter = 1
    problem_counter = 0

    # infinite loop
    while True:
        unix_time_ms = int(time.time()*1000)

        # read sensor data and convert it to line protocol
        data = bme280.sample(bus, SENSOR_ADDRESS)
        line = "sensors_data temperature={},pressure={},humidity={} {}\n".format(data.temperature,
                                                                                 data.pressure,
                                                                                 data.humidity,
                                                                                 unix_time_ms)

        payload += line

        if counter % SENDING_PERIOD == 0:
            try:
                # try to send data to cloud
                r = requests.post(URL, params=corlysis_params, data=payload)
                if r.status_code != 204:
                    raise Exception("data not written")
                payload = ""
            except:
                problem_counter += 1
                print('cannot write to InfluxDB')
                if problem_counter == MAX_LINES_HISTORY:
                    problem_counter = 0
                    payload = ""

        counter += 1

        # wait for selected time
        time_diff_ms = int(time.time()*1000) - unix_time_ms
        print(time_diff_ms)
        if time_diff_ms < READING_DATA_PERIOD_MS:
            time.sleep((READING_DATA_PERIOD_MS - time_diff_ms)/1000.0)


if __name__ == "__main__":
    main()
