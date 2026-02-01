import azure.functions as func
import logging
import json

app = func.FunctionApp(http_auth_level=func.AuthLevel.FUNCTION)

@app.route(route="esp_data")
def http_trigger(req: func.HttpRequest) -> func.HttpResponse:
    logging.info('Python HTTP trigger function processed a request.')

    try:
        # 1. ESP32から送られてきたJSONを取得
        req_body = req.get_json()
        
        # 2. ログに出力（Azureポータルの「ログ」で見れます）
        device_id = req_body.get('device_id')
        temp = req_body.get('temperature')
        humi = req_body.get('humidity')
        
        logging.info(f"Received from {device_id}: Temp={temp}, Humi={humi}")

        # 3. ESP32へレスポンスを返す
        return func.HttpResponse(
            f"Success: Received data from {device_id}",
            status_code=200
        )
    except ValueError:
        return func.HttpResponse(
             "Invalid JSON",
             status_code=400
        )