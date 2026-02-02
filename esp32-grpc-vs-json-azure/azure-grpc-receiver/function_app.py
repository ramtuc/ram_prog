import azure.functions as func
import logging
import json
import sensor_pb2  # 第1回で作ったファイルを同じフォルダに配置！

app = func.FunctionApp(http_auth_level=func.AuthLevel.FUNCTION)

# --- 1. 従来のJSON用（比較用として残す） ---
@app.route(route="esp_data_json")
def http_trigger_json(req: func.HttpRequest) -> func.HttpResponse:
    logging.info('Processing JSON data.')
    try:
        req_body = req.get_json()
        device_id = req_body.get('device_id')
        logging.info(f"[JSON] Received: {req_body}")
        return func.HttpResponse(f"JSON Success: {device_id}", status_code=200)
    except ValueError:
        return func.HttpResponse("Invalid JSON", status_code=400)

# --- 2. 今回のProtobuf用（本命） ---
@app.route(route="esp_data_grpc")
def http_trigger_grpc(req: func.HttpRequest) -> func.HttpResponse:
    logging.info('Processing Protobuf (gRPC) data.')
    try:
        # 生のバイナリデータを取得
        body = req.get_body() 
        
        # Protobufとしてデコード
        sensor_msg = sensor_pb2.SensorData()
        sensor_msg.ParseFromString(body)

        logging.info(f"[gRPC] Decoded: {sensor_msg.device_id}, Temp: {sensor_msg.temperature}")

        return func.HttpResponse(
            f"gRPC Success: Received {len(body)} bytes", 
            status_code=200
        )
    except Exception as e:
        logging.error(f"Decode Error: {e}")
        return func.HttpResponse(f"Decode Failed", status_code=500)