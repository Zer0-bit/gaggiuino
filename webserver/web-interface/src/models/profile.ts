export enum PhaseType {
  FLOW = 'FLOW',
  PRESSURE = 'PRESSURE',
}

export enum CurveStyle {
  EASE_IN= 'EASE_IN',
  EASE_OUT= 'EASE_OUT',
  EASE_IN_OUT= 'EASE_IN_OUT',
  LINEAR= 'LINEAR',
  INSTANT= 'INSTANT',
}

export interface Transition {
  start?: number;
  end: number;
  curve: CurveStyle
  time?: number;
}

export interface PhaseStopConditions {
    time?: number;
    pressureAbove?: number;
    pressureBelow?: number;
    flowAbove?: number;
    flowBelow?: number;
    weight?: number;
    waterPumpedInPhase?: number;
  }

export interface Phase {
  name?: string;
  type: PhaseType;
  target: Transition;
  restriction?: number;
  stopConditions: PhaseStopConditions;
  skip: boolean
  waterTemperature?: number;
}

export interface GlobalStopConditions {
  time?: number,
  weight?: number,
  waterPumped?: number,
}

export interface BrewRecipe {
  coffeeIn?: number;
  coffeeOut?: number;
  ratio?: number;
}

export interface Profile {
  id?: number;
  name: string;
  phases: Array<Phase>;
  globalStopConditions?: GlobalStopConditions ;
  waterTemperature: number;
  recipe?: BrewRecipe;
}

export interface ProfileSummary {
  id: number;
  name: string;
}
