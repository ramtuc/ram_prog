from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI()

# 受信データの型定義
class SensorData(BaseModel):
    temperature: float
    humidity: int
    device_id: str

@app.post("/api/data")
async def receive_data(data: SensorData):
    print("--- 届いたデータ ---")
    print(f"デバイスID: {data.device_id}")
    print(f"温度: {data.temperature} ℃")
    print(f"湿度: {data.humidity} %")
    return {"status": "success", "message": "Data received"}

# uv run app.py で動くように設定
if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)