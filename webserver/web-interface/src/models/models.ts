export enum OperationMode {
    BREW_AUTO = 'BREW_AUTO',
    BREW_MANUAL = 'BREW_MANUAL',
    FLUSH = 'FLUSH',
    DESCALE = 'DESCALE',
    STEAM = 'STEAM',
}

export interface SystemState {
    startupInitFinished: boolean,
    operationMode: OperationMode,
    tofReady: boolean,
    isSteamForgottenON: boolean,
    scalesPresent: boolean,
    timeAlive: number,
    descaleProgress: number,
}

export interface SensorState {
    brewActive: boolean;
    steamActive: boolean;
    hotWaterSwitchState: boolean;
    temperature: number;
    waterTemperature: number;
    pressure: number;
    pumpFlow: number;
    weightFlow: number;
    weight: number;
    waterLevel: number;
}

export interface ShotSnapshot {
    timeInShot: number;
    pressure: number;
    pumpFlow: number;
    weightFlow: number;
    temperature: number;
    shotWeight: number;
    waterPumped: number;
    targetTemperature: number;
    targetPumpFlow: number;
    targetPressure: number;
}

export interface LogMessage {
    source: string,
    log: string,
}

export interface Shot {
  time: number,
  datapoints: Array<ShotSnapshot>,
}
