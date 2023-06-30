import  React, { Component } from "react";
import ReactDOM from "react-dom";
import GaugeLiquid from "react-liquid-gauge";

class GaugeLiquidComponent extends Component {
  constructor(props) {
    super(props);
    this.state = {
      value: props.initialValue || 50
    };
    // startColor = "#6495ed"; // cornflowerblue
    // endColor = "#dc143c"; // crimson
  }

  updateValue = (newValue) => {
    this.setState({ value: newValue });
  };

  render() {
    const radius = 70;

    return (
      <div>
        <div style={{ textAlign: "center", color: "" }}>Water Level</div>
        <GaugeLiquid
          style={{ margin: "0 auto" }}
          width={radius * 2}
          height={radius * 2}
          value={this.state.value}
          percent="%"
          textSize={1}
          textOffsetX={0}
          textOffsetY={0}
          textRenderer={props => {
            const value = Math.round(props.value);
            const radius = Math.min(props.height / 2, props.width / 2);
            const textPixels = (props.textSize * radius) / 2;
            const valueStyle = {
              fontSize: textPixels
            };
            const percentStyle = {
              fontSize: textPixels * 0.6
            };

            return (
              <tspan>
                <tspan className="value" style={valueStyle}>
                  {value}
                </tspan>
                <tspan style={percentStyle}>{props.percent}</tspan>
              </tspan>
            );
          }}
          riseAnimation
          waveAnimation
          waveFrequency={1}
          waveAmplitude={3}
        />
      </div>
    );
  }
}

const root = document.getElementById("root");

if (root !== null) {
  ReactDOM.render(<GaugeLiquidComponent />, root);
}

export default GaugeLiquidComponent;
