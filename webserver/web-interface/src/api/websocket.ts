import { useWebSocket as reactUseWebSocket } from 'react-use-websocket/dist/lib/use-websocket';
import {
  useCallback, useEffect, useState,
} from 'react';
import { Options } from 'react-use-websocket/dist/lib/types';
import useSensorStateStore from '../state/SensorStateStore';
import {
  GaggiaSettings,
  LogMessage, SensorState, ShotSnapshot, SystemState,
} from '../models/models';
import useSystemStateStore from '../state/SystemStateStore';
import useLogMessageStore from '../state/LogStore';
import useShotDataStore from '../state/ShotDataStore';
import useProfileStore from '../state/ProfileStore';
import { Profile } from '../models/profile';
import useSettingsStore from '../state/SettingsStore';

enum WsActionType {
    SensorStateUpdate = 'sensor_data_update',
    ShotSnapshotUpdate = 'shot_data_update',
    LogRecordUpdate = 'log_record',
    SystemStateUpdate = 'sys_state',
    ActiveProfileUpdated ='act_prof_update',
    SettingsUpdated ='settings_update',
}

// Time after which, if we didn't receive any data, the websocket will try to reconnect
const TIMEOUT_INTERVAL = 4000;

const WS_OPTIONS: Options = {
  share: true,
  shouldReconnect: () => true,
  reconnectAttempts: 1000000,
  reconnectInterval: 3,
  retryOnError: true,
};

interface MessageData {
    action: WsActionType,
    data: unknown,
}

const useWebSocket = (url:string) => {
  const { updateLocalSensorState } = useSensorStateStore();
  const { updateLocalSystemState } = useSystemStateStore();
  const { addMessage } = useLogMessageStore();
  const { addShotDatapoint, setShotRunning } = useShotDataStore();
  const { updateLocalActiveProfile } = useProfileStore();
  const { updateLocalSettings } = useSettingsStore();

  const [connected, setConnected] = useState(true);
  const [, setMessageTimeoutId] = useState<NodeJS.Timeout>();
  const { lastJsonMessage, getWebSocket } = reactUseWebSocket(url, WS_OPTIONS, connected);

  // Function to reset connection
  const resetConnection = useCallback(() => {
    const webSocketInstance = getWebSocket();
    if (webSocketInstance && webSocketInstance.readyState === WebSocket.OPEN) {
      setConnected(false);
      setTimeout(() => setConnected(true), 1000);
    }
  }, [getWebSocket]);

  // If no message in {TIMEOUT_INTERVAL} seconds, reset connection
  useEffect(() => {
    let timeoutId: NodeJS.Timeout;

    setMessageTimeoutId((previousTimeoutId) => {
      if (previousTimeoutId) {
        clearTimeout(previousTimeoutId);
      }
      timeoutId = setTimeout(resetConnection, TIMEOUT_INTERVAL);
      return timeoutId;
    });
    return () => clearTimeout(timeoutId);
  }, [lastJsonMessage, resetConnection]);

  useEffect(() => {
    if (!lastJsonMessage) return;

    const messageData = lastJsonMessage as unknown as MessageData;
    switch (messageData.action) {
      case WsActionType.SensorStateUpdate: {
        const sensorState = messageData.data as SensorState;
        updateLocalSensorState(sensorState);
        if (!sensorState.brewActive) {
          setShotRunning(false);
        }
        break;
      }
      case WsActionType.ShotSnapshotUpdate:
        addShotDatapoint(messageData.data as ShotSnapshot);
        break;
      case WsActionType.LogRecordUpdate:
        addMessage(messageData.data as LogMessage);
        break;
      case WsActionType.SystemStateUpdate:
        updateLocalSystemState(messageData.data as SystemState);
        break;
      case WsActionType.ActiveProfileUpdated:
        updateLocalActiveProfile(messageData.data as Profile);
        break;
      case WsActionType.SettingsUpdated:
        updateLocalSettings(messageData.data as GaggiaSettings);
        break;
    }
  }, [lastJsonMessage, updateLocalSettings, updateLocalSensorState,
    updateLocalSystemState, addMessage, addShotDatapoint, updateLocalActiveProfile,
    setShotRunning]);
};

export default useWebSocket;
