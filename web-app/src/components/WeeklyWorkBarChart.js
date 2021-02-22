import React from 'react';
import {Chart, Bar} from 'react-chartjs-2';
import firebase from '.././firebase.js';
import 'chartjs-plugin-annotation';
import 'chartjs-plugin-deferred';

Chart.defaults.global.defaultFontColor = '#fafafa';
Chart.defaults.global.defaultfontFamily = "'Monserrat', sans-serif";

var dta = [3, 1.5, 6, 5, 8, 2, 4]

const WeeklyWorkBarChart = ({
  targetHours,
  timeData
}) => {

  //console.log(timeData)
  //Set data and target line
  var targetLine = targetHours 

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