import React from 'react';

const StatusBase = ({
  className,
  timeData,
  awayDesk
}) => {

  /*if(has been greater than 5 mins since timeData[end]){
  display that the device is off - stop recording data?
  }
  */

  for (let i = 0; i < awayDesk.length; i++) {
    //don't add point if away from desk
    if (awayDesk[i] == false){
    }
  }

  var status = "Off"
  const now = new Date()
  const nowInUnixSecs = Math.round(now.getTime() / 1000)
   if(awayDesk[awayDesk.length - 1] == true){
    status = "Away From Desk"
  }
  else if((nowInUnixSecs - timeData[timeData.length - 1])/60 < 5){
    status = "At Work"
  }

  return (
    <div className={className}>
      <div className="table">
        <ul>
          <li> Current Status:</li>
          <li className="status">{status}</li>
        </ul>
      </div>
    </div>
  );
}

export default StatusBase;

