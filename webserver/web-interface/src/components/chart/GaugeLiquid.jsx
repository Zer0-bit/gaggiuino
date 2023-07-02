import  React, { Component } from "react";
import GaugeLiquid from "react-liquid-gauge";

function GaugeLiquidComponent({value=0, radius=50}) {

  return (
    <div>
      <div style={{ textAlign: "center", fontSize: "13px", color: "gray" }}>Water Level</div>
      <br></br>
      <GaugeLiquid
        style={{ margin: "0 auto" }}
        width={radius / 3.5}
        height={radius / 3.5}
        value={value}
        percent="%"
        textSize={1.3}
        textOffsetX={0}
        textOffsetY={0}
        riseAnimation
        waveAnimation
        waveFrequency={1}
        waveAmplitude={3}
        textStyle={{
            fill: '#178bca',
            fontFamily: 'Arial'
        }}
        waveTextStyle={{
            fill: '#178bca',
            fontFamily: 'Arial'
        }}
      />
    </div>
  );
};

export default GaugeLiquidComponent;
