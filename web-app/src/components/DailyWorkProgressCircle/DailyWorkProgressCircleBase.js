import React, { useState, useEffect } from 'react';

const INITIAL_OFFSET = 25;
const circleConfig = {
    viewBox: '0 0 38 38',
    x: '19',
    y: '19',
    radio: '15.91549430918954'
  };

const DailyWorkProgressCircleBase = ({
    className,
    loadRingColour,
    loadRingWidth,
    innerText,
    timeData,
    targetHours,
    fullRingWidth,
    fullRingColour,
    awayDesk
  }) => {

  var hoursWorked = 0
  var previousTime = timeData[0]
  for (let i = 0; i < timeData.length; i++) {
   //don't add point if away from desk
    if (awayDesk[i] == false){
      if(awayDesk[i-1] == true){
        previousTime = timeData[i]  
      }
      else{
        hoursWorked += (timeData[i]-previousTime)
        previousTime = timeData[i]  
      }
    }
  }
  
  hoursWorked = (hoursWorked/3600).toFixed(1) 

  var percentage = 100*(hoursWorked/targetHours)
  var reviewText = 0
  if (hoursWorked > targetHours) {
      reviewText = (hoursWorked - targetHours).toFixed(1) + " h more than target, good job!"
  } else if (hoursWorked >= targetHours) {
      reviewText = "Hours of work target met, well done!" 
  } else {
      reviewText = (targetHours - hoursWorked).toFixed(1) + " h less than target, keep going!"
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

        <circle
          className="load-ring"
          cx={circleConfig.x}
          cy={circleConfig.y}
          r={circleConfig.radio}
          fill="transparent"
          stroke={loadRingColour}
          strokeWidth={loadRingWidth}
          strokeDasharray={`${percentage} ${100 - percentage}`}
          strokeDashoffset={INITIAL_OFFSET}
        />

        <g className="chart-text">
          <text x="50%" y="50%" className="chart-number">
            {hoursWorked}/{targetHours}
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

export default DailyWorkProgressCircleBase;