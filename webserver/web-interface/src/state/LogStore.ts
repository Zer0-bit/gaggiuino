import { create } from 'zustand';
import { devtools } from 'zustand/middleware';
import { LogMessage } from '../models/models';

const MAX_LOG_MESSAGES = 200;

interface LogMessageStore {
  logs: Array<LogMessage>,
  addMessage: (message: LogMessage) => void,
}

const useLogMessageStore = create<LogMessageStore>()(
  devtools(
    (set) => ({
      logs: [],
      addMessage: (message: LogMessage) => set((state) => {
        let { logs } = state;
        if (logs.length >= MAX_LOG_MESSAGES) {
          logs = logs.slice(0, Math.max(0, state.logs.length - MAX_LOG_MESSAGES + 1));
        }
        logs.push(message);
        return { logs };
      }),
    }),
  ),
);

export default useLogMessageStore;
