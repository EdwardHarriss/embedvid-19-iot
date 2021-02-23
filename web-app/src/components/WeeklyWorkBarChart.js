import React from 'react';
import {Chart, Bar} from 'react-chartjs-2';
import firebase from '.././firebase.js';
import 'chartjs-plugin-annotation';
import 'chartjs-plugin-deferred';

Chart.defaults.global.defaultFontColor = '#fafafa';
Chart.defaults.global.defaultfontFamily = "'Monserrat', sans-serif";

const WeeklyWorkBarChart = ({
  targetHours,
  timeData,
  awayDesk
}) => {

  //Set data and target line
  var targetLine = targetHours
  const now = new Date()
  var dta = []
  //Total Hours worked each day
  var MonHours=0, TueHours=0, WedHours=0, ThuHours=0, FriHours=0, SatHours=0, SunHours=0
  var previousTime=timeData[0]
  for (let i = 0; i < timeData.length; i++) {
    let DataDate = new Date(timeData[i] * 1000)
    if(DataDate.getDay() == 1){
      if (awayDesk[i] == false){
        if(awayDesk[i-1] == true){
          previousTime = timeData[i]  
        }
        else{
          MonHours += (timeData[i]-previousTime)
          previousTime = timeData[i]  
        }
      }
      dta[0] = (MonHours/3600).toFixed(1)
    }
    if(DataDate.getDay() == 2){
      if (awayDesk[i] == false){
        if(awayDesk[i-1] == true){
          previousTime = timeData[i]  
        }
        else{
          TueHours += (timeData[i]-previousTime)
          previousTime = timeData[i]  
        }
      }
      dta[1] = (TueHours/3600).toFixed(1)
    }
    if(DataDate.getDay() == 3){
      if (awayDesk[i] == false){
        if(awayDesk[i-1] == true){
          previousTime = timeData[i]  
        }
        else{
          WedHours += (timeData[i]-previousTime)
          previousTime = timeData[i]  
        }
      }
      dta[2] = (WedHours/3600).toFixed(1)
    }
    if(DataDate.getDay() == 4){
      if (awayDesk[i] == false){
        if(awayDesk[i-1] == true){
          previousTime = timeData[i]  
        }
        else{
          ThuHours += (timeData[i]-previousTime)
          previousTime = timeData[i]  
        }
      }
      dta[3] = (ThuHours/3600).toFixed(1)
    }
    //Friday
    if(DataDate.getDay() == 5){
      if (awayDesk[i] == false){
        if(awayDesk[i-1] == true){
          previousTime = timeData[i]  
        }
        else{
          FriHours += (timeData[i]-previousTime)
          previousTime = timeData[i]  
        }
      }
      dta[4] = (FriHours/3600).toFixed(1)
    }
    //Saturday
    if(DataDate.getDay() == 6){
      if (awayDesk[i] == false){
        if(awayDesk[i-1] == true){
          previousTime = timeData[i]  
        }
        else{
          SatHours += (timeData[i]-previousTime)
          previousTime = timeData[i]  
        }
      }
      dta[5] = (SatHours/3600).toFixed(1)
    }
    //Sunday
    if(DataDate.getDay() == 0){
      if (awayDesk[i] == false){
        if(awayDesk[i-1] == true){
          previousTime = timeData[i]  
        }
        else{
          SunHours += (timeData[i]-previousTime)
          previousTime = timeData[i]  
        }
      }
      dta[6] = (SunHours/3600).toFixed(1)
    }
  }

  //set bar colours
  var colours = []
  for (let i = 0; i < dta.length; i++) {
    var colour = '#6d78ad'
    if(dta[i] >= targetLine){
      colour = '#51cda0'
    }
    colours[i] = colour
  }

  return (
    <div>
      <Bar
        data={{
          labels: ['M', 'T', 'W', 'T',
                  'F', 'S', 'S'],  
          datasets: [
            {
            label: 'Hours Worked',
            backgroundColor: colours,
            borderColor: 'rgba(0, 0, 0, 0)',
            borderWidth: 1,
            data: dta//timeData
            }
          ]
        }}
        options={{
          responsive:true,
          legend:{
            display:false,
          },
          scales:{
            yAxes:[{
              gridLines:{
                zeroLineWidth:3
              },
              ticks:{
                fontSize:14,
                fontColor:'#98999e',
                stepSize:1,
                beginAtZero:true
              }
            }],
            xAxes:[{
              gridLines:{
                zeroLineWidth:3
              },
              ticks:{
                fontSize:14,
                fontColor:'#98999e'
              }
            }]
          },
          annotation:{
            annotations:[{
              borderColor:'#98999e',
              borderWidth:2,
              borderDash:[10],
              mode:'horizontal',
              type:'line',
              value:targetLine,
              scaleID:'y-axis-0'
            }]
          },
          plugins:{
            deferred: {
              yOffset:'30%',
              delay:250     
            }
          }
        }}
      />
    </div>
  );
}


export default WeeklyWorkBarChart