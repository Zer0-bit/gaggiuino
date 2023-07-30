import { create } from 'zustand';
import { devtools } from 'zustand/middleware';
import { Notification } from '../models/models';

interface NotificationStore {
  latestNotification?: Notification,
  updateLatestNotification: (notification: Notification) => void,
}

const useNotificationStore = create<NotificationStore>()(
  devtools(
    (set) => ({
      latestNotification: undefined,
      updateLatestNotification: (notification: Notification) => set(() => ({ latestNotification: notification })),
    }),
  ),
);

export default useNotificationStore;
