#pip install requests psutil
import time
import requests
import psutil
import argparse

# constants
URL = 'https://corlysis.com:8086/write'
READING_DATA_PERIOD_MS = 1000.0
SENDING_PERIOD = 5
MAX_LINES_HISTORY = 1000


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("db", help="database name")
    parser.add_argument("token", help="secret token")
    args = parser.parse_args()

    corlysis_params = {"db": args.db, "u": "token", "p": args.token, "precision": "ms"}

    # initialization
    payload = ""
    counter = 1
    problem_counter = 0

    # infinite loop
    while True:
        unix_time_ms = int(time.time()*1000)

        # read sensor data and convert it to line protocol
        line = "server_data cpu_percent={},free_memory={} {}\n".format(psutil.cpu_percent(interval=None),
                                                                       psutil.virtual_memory().free,
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

