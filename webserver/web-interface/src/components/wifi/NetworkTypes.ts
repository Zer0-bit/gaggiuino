export type WifiStatus = {
  status: 'connected' | 'disconnected';
  ssid: string,
  ip: string,
}

export type Network = {
  ssid: string;
  rssi: number;
  secured: boolean;
}
