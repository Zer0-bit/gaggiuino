import axios, { AxiosRequestConfig } from 'axios';
import { SensorState } from '../../models/models';

const config:AxiosRequestConfig = {
  timeout: 4000,
};

async function getTare(): Promise<SensorState> {
  return axios.get('/api/sensor-state', config)
    .then(({ data }) => data)
    .then(({ sensorState }) => sensorState);
}

// export async function updateTare(operationMode: SensorState): Promise<void> {
//   return axios.put('/api/sensor-state', { sensorState }, config);
// }
