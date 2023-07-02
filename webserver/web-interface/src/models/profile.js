/* eslint-disable max-classes-per-file */

function notDefined(value) {
  return (value === null || value === undefined);
}

export const PhaseTypes = Object.freeze({
  FLOW: Symbol('FLOW'),
  PRESSURE: Symbol('PRESSURE'),
});

export function parsePhaseType(phaseTypeObj) {
  if (notDefined(phaseTypeObj)) {
    return undefined;
  }
  const phaseType = PhaseTypes[phaseTypeObj];

  if (!phaseType) {
    throw Error(`${phaseTypeObj} not one of PhaseTypes{${Object.keys(PhaseTypes).join(',')}}`);
  }
  return phaseType;
}

export const CurveStyles = Object.freeze({
  EASE_IN: Symbol('EASE_IN'),
  EASE_OUT: Symbol('EASE_OUT'),
  EASE_IN_OUT: Symbol('EASE_IN_OUT'),
  LINEAR: Symbol('LINEAR'),
  INSTANT: Symbol('INSTANT'),
});

export function parseCurveStyle(curveStyleObj) {
  if (notDefined(curveStyleObj)) {
    return undefined;
  }
  const curveStyle = CurveStyles[curveStyleObj];

  if (!curveStyle) {
    throw Error(`${curveStyle} not one of CurveStyles{${Object.keys(CurveStyles).join(',')}}`);
  }
  return curveStyle;
}

export class Transition {
  constructor(start, end, curve, time) {
    if (start === null || start === undefined) {
      throw Error('start is required for Transition');
    }
    this.start = start;
    this.end = end;
    this.curve = curve;
    this.time = time;
  }

  static parse(obj) {
    return new Transition(obj.start, obj.end, parseCurveStyle(obj.curve), obj.time);
  }
}

export class PhaseStopConditions {
  constructor(
    time,
    pressureAbove,
    pressureBelow,
    flowAbove,
    flowBelow,
    weight,
    waterPumpedInPhase,
  ) {
    this.time = time;
    this.pressureAbove = pressureAbove;
    this.pressureBelow = pressureBelow;
    this.flowAbove = flowAbove;
    this.flowBelow = flowBelow;
    this.weight = weight;
    this.waterPumpedInPhase = waterPumpedInPhase;
  }

  static parse(obj) {
    return new PhaseStopConditions(
      obj.time,
      obj.pressureAbove,
      obj.pressureBelow,
      obj.flowAbove,
      obj.flowBelow,
      obj.weight,
      obj.waterPumpedInPhase,
    );
  }
}

export class Phase {
  constructor(type, target, restriction, stopConditions) {
    if (notDefined(type) || notDefined(target)) {
      throw Error('type and target are requred for Phase');
    }
    this.type = type;
    this.target = target;
    this.restriction = restriction;
    this.stopConditions = stopConditions;
  }

  static parse(obj) {
    return new Phase(
      parsePhaseType(obj.type),
      Transition.parse(obj.target),
      obj.restriction,
      obj.stopConditions ? PhaseStopConditions.parse(obj.stopConditions) : undefined,
    );
  }
}

export class GlobalStopConditions {
  constructor(
    time,
    weight,
    waterPumped,
  ) {
    this.time = time;
    this.weight = weight;
    this.waterPumped = waterPumped;
  }

  static parse(obj) {
    return new GlobalStopConditions(
      obj.time,
      obj.weight,
      obj.waterPumped,
    );
  }
}

export class Profile {
  constructor(phases, globalStopConditions) {
    this.phases = phases;
    this.globalStopConditions = globalStopConditions;
  }

  static parse(obj) {
    return new Profile(
      Array.isArray(obj.phases) ? obj.phases.map((phase) => Phase.parse(phase)) : [],
      obj.globalStopConditions ? GlobalStopConditions.parse(obj.globalStopConditions) : undefined,
    );
  }
}
