import React from 'react';
import {Chart, Bar} from 'react-chartjs-2';
import firebase from '.././firebase.js';
import 'chartjs-plugin-annotation';
import 'chartjs-plugin-deferred';

Chart.defaults.global.defaultFontColor = '#fafafa';
Chart.defaults.global.defaultfontFamily = "'Monserrat', sans-serif";


const WeeklyWorkBarChart = ({
  targetHours,
  timeData
}) => {

  //Set data and target line
  var targetLine = targetHours
  var dta = [3, 1.5, 6, 5, 8, 2, 4]

  //calculate hours and add it to the right day
  var hoursWorkedNonRounded = (timeData[timeData.length - 1] - timeData[0])/3600
  var hoursWorked = hoursWorkedNonRounded.toFixed(1)
  var currentDate = new Date()
  //var day = currentDate.getDay()
  //dta[day] = hoursWorked
  //var day = 2
  //dta[day] = hoursWorked
  //console.log(currentDate.getHours())
  //console.log(currentDate.getMinutes())
  //console.log(currentDate.getSeconds())
  //push the day's data to database if end of day
  /*if(currentDate == 12 && currentDate.getMinutes() == 10 && currentDate.getSeconds() == 55){
    const itemsRef = firebase.database().ref('weeklyHoursData')
    const item = {
      day: hoursWorked,
    }
    itemsRef.push(item)
  }*/

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
            data: dta
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