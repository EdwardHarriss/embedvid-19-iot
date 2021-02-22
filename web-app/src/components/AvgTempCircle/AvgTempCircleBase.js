import React from 'react';

const INITIAL_OFFSET = 25;
const circleConfig = {
    viewBox: '0 0 38 38',
    x: '19',
    y: '19',
    radio: '15.91549430918954'
  };

const AvgTempCircleBase = ({
  className,
  textColor,
  innerText,
  tempData,
  fullRingWidth,
  fullRingColour
}) => {
  var sum = tempData.reduce((a, b) => a + b, 0);
  var avgTempNonRounded = (sum / tempData.length) || 0;
  var avgTemp = avgTempNonRounded.toFixed(1)
  
  var reviewText = "Your work environment is"
  if (avgTemp > 23) {
      reviewText += " above the optimum temperature, try and cool it down!"
      fullRingColour = "#e6214f"
  } else if (avgTemp < 20) {
      reviewText += " below the optimum temperature, try and warm it up!"
      fullRingColour = "#6d78ad"
  } else {
      reviewText += " at the optimum temperature, nice!"
  }
  return (
    <figure className={className}>
      <svg viewBox={circleConfig.viewBox}>
        <circle
          className="full-ring"
          cx={circleConfig.x}
          cy={circleConfig.y}
          r={circleConfig.radio}
          fill="transparent"
          stroke={fullRingColour}
          strokeWidth={fullRingWidth}
        />

        <g className="chart-text">
          <text x="50%" y="50%" className="chart-number">
            {avgTemp+'\xB0'}
          </text>
          <text x="50%" y="50%" className="chart-label">
            {innerText}
          </text>
        </g>
      </svg>
      <p className="review-text"> {reviewText} </p>
    </figure>
  );
};
  
export default AvgTempCircleBase;