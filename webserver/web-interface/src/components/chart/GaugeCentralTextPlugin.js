const GaugeCentralTextPlugin = {
  id: 'center',
  afterDraw(chart) {
    if (chart.config.options.plugins && chart.config.options.plugins.center) {
      // Get ctx from string
      const { ctx } = chart;

      // Get options from the center object in options
      const centerConfig = chart.config.options.plugins.center;
      const fontStyle = centerConfig.fontStyle || 'Arial';
      const txt = centerConfig.text;
      const color = centerConfig.color || '#000';
      const maxFontSize = centerConfig.maxFontSize || 75;
      const sidePadding = centerConfig.sidePadding || 20;
      // eslint-disable-next-line no-underscore-dangle
      const { innerRadius } = chart._metasets[chart._metasets.length - 1].data[0];
      const sidePaddingCalculated = (sidePadding / 100) * (innerRadius * 2);
      // Start with a base font of 30px
      ctx.font = `30px ${fontStyle}`;
      //   const outerRadius = chart._metasets[0].data[0].outerRadius;

      // Get the width of the string and also the width of the element minus 10 to
      // give it 5px side padding
      const stringWidth = ctx.measureText(txt).width;
      const elementWidth = (innerRadius * 2) - sidePaddingCalculated;

      // Find out how much the font can grow in width.
      const widthRatio = elementWidth / stringWidth;
      const newFontSize = Math.floor(30 * widthRatio);
      const elementHeight = (innerRadius * 2);

      // Pick a new font size so it will not be larger than the height of label.
      let fontSizeToUse = Math.min(newFontSize, elementHeight, maxFontSize);

      let { minFontSize } = centerConfig;
      const lineHeight = centerConfig.lineHeight || 25;
      let wrapText = false;

      if (minFontSize === undefined) {
        minFontSize = 20;
      }

      if (minFontSize && fontSizeToUse < minFontSize) {
        fontSizeToUse = minFontSize;
        wrapText = true;
      }

      // Set font settings to draw it correctly.
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      const centerX = ((chart.chartArea.left + chart.chartArea.right) / 2);
      let centerY = ((chart.chartArea.top + chart.chartArea.bottom) / 2);
      ctx.font = `${fontSizeToUse}px ${fontStyle}`;
      ctx.fillStyle = color;

      if (!wrapText) {
        ctx.fillText(txt, centerX, centerY);
        return;
      }

      const words = txt.split(' ');
      let line = '';
      const lines = [];

      // Break words up into multiple lines if necessary
      // eslint-disable-next-line no-plusplus
      for (let n = 0; n < words.length; n++) {
        const testLine = `${line + words[n]} `;
        const metrics = ctx.measureText(testLine);
        const testWidth = metrics.width;
        if (testWidth > elementWidth && n > 0) {
          lines.push(line);
          line = `${words[n]} `;
        } else {
          line = testLine;
        }
      }

      // Move the center up depending on line height and number of lines
      centerY -= (lines.length / 2) * lineHeight;

      // eslint-disable-next-line no-plusplus
      for (let n = 0; n < lines.length; n++) {
        ctx.fillText(lines[n], centerX, centerY);
        centerY += lineHeight;
      }
      // Draw text in center
      ctx.fillText(line, centerX, centerY);
    }
  },
};

export default GaugeCentralTextPlugin;
