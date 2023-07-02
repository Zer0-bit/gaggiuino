import axios from 'axios';

export async function getWifiStatus() {
  return axios.get('/api/wifi/status')
    .then(({ data }) => data);
}

export async function getAvailableNetworks() {
  return axios.get('/api/wifi/networks')
    .then(({ data }) => {
      const networks = data.filter((network) => network.ssid !== null && network.ssid.length > 0);
      const networksUniqueByKey = [...new Map(networks.map((item) => [item.ssid, item])).values()];
      return networksUniqueByKey;
    });
}

export async function disconnectFromWifi() {
  return axios.delete('/api/wifi/selected-network');
}

export async function connectToWifi({ ssid, pass }) {
  return axios.put('/api/wifi/selected-network', { ssid, pass }, {
    timeout: 10000,
  });
}

export async function refrehNetworks() {
  return axios.delete('/api/wifi/networks');
}
