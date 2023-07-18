import axios from 'axios';
import { OperationMode } from '../../models/models';

export async function getOperationMode(): Promise<OperationMode> {
  return axios.get('/api/system-state/operation-mode')
    .then(({ data }) => data)
    .then(({ operationMode }) => operationMode);
}

export async function updateOperationMode(operationMode: OperationMode): Promise<void> {
  return axios.put('/api/system-state/operation-mode', { operationMode });
}
