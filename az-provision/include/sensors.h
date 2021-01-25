
struct SensorRead{
  float temperature;
  float humidity;
  int light; 
};

void initializeSensors();

SensorRead getSensorData();
