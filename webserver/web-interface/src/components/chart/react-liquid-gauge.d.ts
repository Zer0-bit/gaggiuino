import React from 'react';

interface GradientStop {
  offset: string;
  stopColor: string;
  stopOpacity: string;
}

interface LiquidFillGaugeProps {
  id?: string;
  width?: number;
  height?: number;
  value?: number;
  percent?: string | React.ReactNode;
  textSize?: number;
  textOffsetX?: number;
  textOffsetY?: number;
  textRenderer?: (percentage: number) => React.ReactNode;
  riseAnimation?: boolean;
  riseAnimationTime?: number;
  riseAnimationEasing?: string;
  riseAnimationOnProgress?: () => void;
  riseAnimationOnComplete?: () => void;
  waveAnimation?: boolean;
  waveAnimationTime?: number;
  waveAnimationEasing?: string;
  waveAmplitude?: number;
  waveFrequency?: number;
  gradient?: boolean;
  gradientStops?: GradientStop[] | React.ReactNode;
  onClick?: () => void;
  innerRadius?: number;
  outerRadius?: number;
  margin?: number;
  circleStyle?: React.CSSProperties;
  waveStyle?: React.CSSProperties;
  textStyle?: React.CSSProperties;
  waveTextStyle?: React.CSSProperties;
}

declare const LiquidFillGauge: React.ComponentType<LiquidFillGaugeProps>;

export default LiquidFillGauge;
