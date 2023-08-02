import axios, { AxiosRequestConfig } from 'axios';
import { OperationMode, SystemState } from '../../models/models';

const config:AxiosRequestConfig = {
  timeout: 4000,
};

export async function getSystemState(): Promise<SystemState> {
  return axios.get('/api/system-state', config)
    .then(({ data }) => data);
}

export async function updateOperationMode(operationMode: OperationMode): Promise<SystemState> {
  return axios.put('/api/system-state/operation-mode', { operationMode }, config)
    .then(({ data }) => data);
}

export async function updateTarePending(tarePending: boolean): Promise<SystemState> {
  return axios.put('/api/system-state/tare-pending', { tarePending }, config)
    .then(({ data }) => data);
}
