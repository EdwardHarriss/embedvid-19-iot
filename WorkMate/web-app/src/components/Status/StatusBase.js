import React from 'react';

const StatusBase = ({
  className,
  timeData,
  awayDesk
}) => {
  
  var status = "n/a"
  var clr = "#98999e"
  const now = new Date()
  const nowInUnixSecs = Math.round(now.getTime() / 1000)
  if((nowInUnixSecs - timeData[timeData.length - 1])/60 > 5 || timeData.length == 0){
    status = "Off"
    clr = "#98999e"
  }
  else{
    var status = "At Work"
    var clr = "#51cda0"
  }

  if(awayDesk[awayDesk.length - 1] == true && status != "Off"){
    status = "Away From Desk"
    clr = "#e6214f"
  }

  return (
    <div className={className}>
      <h4 className="title"> Current Status: </h4>
      <p className="status" style={{color: clr}}>{status}</p>
    </div>
  );
}

export default StatusBase;

