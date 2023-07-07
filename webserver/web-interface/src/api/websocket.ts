import { useEffect } from 'react';
import { useWebSocket as reactUseWebSocket } from 'react-use-websocket/dist/lib/use-websocket';
import useSensorStateStore from '../state/SensorStateStore';
import {
  LogMessage, SensorState, ShotSnapshot, SystemState,
} from '../models/models';
import useSystemStateStore from '../state/SystemStateStore';
import useLogMessageStore from '../state/LogStore';
import useShotDataStore from '../state/ShotDataStore';
import useProfileStore from '../state/ProfileStore';
import { Profile } from '../models/profile';

enum WsActionType {
    SensorStateUpdate = 'sensor_data_update',
    ShotSnapshotUpdate = 'shot_data_update',
    LogRecordUpdate = 'log_record',
    SystemStateUpdate = 'sys_state',
    ActiveProfileUpdated ='act_prof_update',
    SettingsUpdated ='settings_update',
}

interface MessageData {
    action: WsActionType,
    data: unknown,
}

const useWebSocket = (url:string) => {
  const { updateSensorState } = useSensorStateStore();
  const { updateSystemState } = useSystemStateStore();
  const { addMessage } = useLogMessageStore();
  const { addShotDatapoint } = useShotDataStore();
  const { setLocalActiveProfile } = useProfileStore();

  const { lastJsonMessage } = reactUseWebSocket(url, {
    share: true,
    shouldReconnect: () => true,
    reconnectAttempts: 1000,
    reconnectInterval: 1,
    retryOnError: true,
  });

  useEffect(() => {
    if (!lastJsonMessage) return;

    const messageData = lastJsonMessage as unknown as MessageData;
    switch (messageData.action) {
      case WsActionType.SensorStateUpdate:
        updateSensorState(messageData.data as SensorState);
        break;
      case WsActionType.ShotSnapshotUpdate:
        addShotDatapoint(messageData.data as ShotSnapshot);
        break;
      case WsActionType.LogRecordUpdate:
        addMessage(messageData.data as LogMessage);
        break;
      case WsActionType.SystemStateUpdate:
        updateSystemState(messageData.data as SystemState);
        break;
      case WsActionType.ActiveProfileUpdated:
        setLocalActiveProfile(messageData.data as Profile);
        break;
      case WsActionType.SettingsUpdated:
        // TODO: implement setting it in the store
        break;
    }
  }, [lastJsonMessage, updateSensorState, updateSystemState, addMessage, addShotDatapoint, setLocalActiveProfile]);
};

export default useWebSocket;
