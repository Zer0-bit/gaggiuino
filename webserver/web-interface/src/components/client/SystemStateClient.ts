import axios, { AxiosRequestConfig } from 'axios';
import { OperationMode } from '../../models/models';

const config:AxiosRequestConfig = {
  timeout: 4000,
};

export async function getOperationMode(): Promise<OperationMode> {
  return axios.get('/api/system-state/operation-mode', config)
    .then(({ data }) => data)
    .then(({ operationMode }) => operationMode);
}

export async function updateOperationMode(operationMode: OperationMode): Promise<void> {
  return axios.put('/api/system-state/operation-mode', { operationMode }, config);
}
