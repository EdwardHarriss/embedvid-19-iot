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
    speed
  }) => {
  //calculating hours worked
  //CurrentHrsWorked = (this.state.items[this.state.items.length - 1].time - this.state.items[0].time)/3600
  var hoursWorkedNonRounded = (timeData[timeData.length - 1] - timeData[0])/3600
  var hoursWorked = hoursWorkedNonRounded.toFixed(1)
  //animating ring progress  
  const [progressBar, setProgressBar] = useState(0);
  const pace = percentage / speed;
  const updatePercentage = () => {
    setTimeout(() => {
      setProgressBar(progressBar + 0.4);
      }, pace);
    };

    useEffect(() => {
      if (percentage > 0) updatePercentage();
    }, [percentage]);

    useEffect(() => {
      if (progressBar < percentage) updatePercentage();
    }, [progressBar]);
    
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