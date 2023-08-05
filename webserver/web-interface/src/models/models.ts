export enum OperationMode {
    BREW_AUTO = 'BREW_AUTO',
    BREW_MANUAL = 'BREW_MANUAL',
    FLUSH = 'FLUSH',
    FLUSH_AUTO = 'FLUSH_AUTO',
    DESCALE = 'DESCALE',
    STEAM = 'STEAM',
}

export function areOperationModesEqual(mode1: OperationMode, mode2: OperationMode): boolean {
  return mode1 === mode2;
}

export interface SystemState {
    startupInitFinished: boolean,
    operationMode: OperationMode,
    tofReady: boolean,
    isSteamForgottenON: boolean,
    scalesPresent: boolean,
    timeAlive: number,
    descaleProgress: number,
    tarePending: boolean,
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
  datapoints: ShotSnapshot[],
}

export interface BoilerSettings {
  steamSetPoint: number;
  offsetTemp: number;
  hpwr: number;
  mainDivider: number;
  brewDivider: number;
}

export function areBoilerSettingsEqual(settings1: BoilerSettings, settings2: BoilerSettings): boolean {
  return settings1.steamSetPoint === settings2.steamSetPoint
      && settings1.offsetTemp === settings2.offsetTemp
      && settings1.hpwr === settings2.hpwr
      && settings1.mainDivider === settings2.mainDivider
      && settings1.brewDivider === settings2.brewDivider;
}

export interface SystemSettings {
  pumpFlowAtZero: number;
  scalesF1: number;
  scalesF2: number;
  lcdSleep: number;
  warmupState: boolean;
}

export function areSystemSettingsEqual(settings1: SystemSettings, settings2: SystemSettings): boolean {
  return settings1.pumpFlowAtZero === settings2.pumpFlowAtZero
      && settings1.scalesF1 === settings2.scalesF1
      && settings1.scalesF2 === settings2.scalesF2
      && settings1.lcdSleep === settings2.lcdSleep
      && settings1.warmupState === settings2.warmupState;
}

export interface BrewSettings {
  basketPrefill: boolean;
  homeOnShotFinish: boolean;
  brewDeltaState: boolean;
}

export function areBrewSettingsEqual(settings1: BrewSettings, settings2: BrewSettings): boolean {
  return settings1.basketPrefill === settings2.basketPrefill
      && settings1.homeOnShotFinish === settings2.homeOnShotFinish
      && settings1.brewDeltaState === settings2.brewDeltaState;
}

export interface LedColor {
  R: number;
  G: number;
  B: number;
}

export function areLedColorsEqual(color1: LedColor, color2: LedColor): boolean {
  return color1.R === color2.R
      && color1.G === color2.G
      && color1.B === color2.B;
}

export interface LedSettings {
  state: boolean;
  disco: boolean;
  color: LedColor
}

export function areLedSettingsEqual(settings1: LedSettings, settings2: LedSettings): boolean {
  return settings1.state === settings2.state
      && settings1.disco === settings2.disco
      && areLedColorsEqual(settings1.color, settings2.color);
}

export interface ScalesSettings {
  forcePredictive: boolean;
  hwScalesEnabled: boolean;
  hwScalesF1: number;
  hwScalesF2: number;
  btScalesEnabled: boolean;
  btScalesAutoConnect: boolean;
}

export function areScalesSettingsEqual(settings1: ScalesSettings, settings2: ScalesSettings): boolean {
  return settings1.forcePredictive === settings2.forcePredictive
      && settings1.hwScalesEnabled === settings2.hwScalesEnabled
      && settings1.hwScalesF1 === settings2.hwScalesF1
      && settings1.hwScalesF2 === settings2.hwScalesF2
      && settings1.btScalesEnabled === settings2.btScalesEnabled
      && settings1.btScalesAutoConnect === settings2.btScalesAutoConnect;
}

export interface GaggiaSettings {
  boiler: BoilerSettings;
  system: SystemSettings;
  brew: BrewSettings;
  led: LedSettings;
  scales: ScalesSettings;
}

export function areGaggiaSettingsEqual(settings1: GaggiaSettings, settings2: GaggiaSettings): boolean {
  return areBoilerSettingsEqual(settings1.boiler, settings2.boiler)
        && areSystemSettingsEqual(settings1.system, settings2.system)
        && areBrewSettingsEqual(settings1.brew, settings2.brew)
        && areLedSettingsEqual(settings1.led, settings2.led)
        && areScalesSettingsEqual(settings1.scales, settings2.scales);
}

export enum NotificationType {
  INFO = 'INFO',
  SUCCESS = 'SUCCESS',
  ERROR = 'ERROR',
  WARN = 'WARN',
}

export interface Notification {
  type: NotificationType,
  message: string,
}

export enum DescalingState {
  IDLE='IDLE',
  PHASE1='PHASE1',
  PHASE2='PHASE2',
  PHASE3='PHASE3',
  FINISHED='FINISHED',
}

export interface DescalingProgress {
  state: DescalingState;
  time: number; // (time in millis)
  progress: number; // 0-100
}

export interface BleScales {
  name: string;
  address: string;
}
