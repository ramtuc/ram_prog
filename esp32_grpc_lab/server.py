from flask import Flask, request
import sensor_pb2

app = Flask(__name__)

@app.route('/data', methods=['POST'])
def receive_data():
    # 1. 生のバイナリデータを取得
    binary_data = request.data
    print(f"\n--- Received Binary (Size: {len(binary_data)} bytes) ---")
    print(f"Hex: {binary_data.hex(' ')}")

    # 2. Protobufとしてデコード
    try:
        sensor_msg = sensor_pb2.SensorData()
        sensor_msg.ParseFromString(binary_data)

        # 3. デコード結果を表示
        print(f"Device ID: {sensor_msg.device_id}")
        print(f"Temp:      {sensor_msg.temperature:.1f} °C")
        print(f"Humidity:  {sensor_msg.humidity} %")
        print("-" * 30)
    except Exception as e:
        print(f"Decode Error: {e}")

    return "Data Received", 200

if __name__ == '__main__':
    # 0.0.0.0 で待受けることで外部（ESP32）からのアクセスを許可
    print("Starting gRPC Local Receiver...")
    app.run(host='0.0.0.0', port=5000)