import React from 'react';
import {Chart, Bar} from 'react-chartjs-2';
import firebase from '.././firebase.js';
import 'chartjs-plugin-annotation';
import 'chartjs-plugin-deferred';

Chart.defaults.global.defaultFontColor = '#fafafa';
Chart.defaults.global.defaultfontFamily = "'Monserrat', sans-serif";

const WeeklyPostureBarChart = ({
  distanceData,
  timeData,
  awayDesk
}) => {


  //set empty data array
  var BarData = []
  const UpperLine = 1000
  const LowerLine = 500
  const MidPoint = 750
  //Average Temperature each day
  var MonDistanceSum=0, TueDistanceSum=0, WedDistanceSum=0, ThuDistanceSum=0, FriDistanceSum=0, SatDistanceSum=0, SunDistanceSum=0
  var MonDataLength=0, TueDataLength=0, WedDataLength=0, ThuDataLength=0, FriDataLength=0, SatDataLength=0, SunDataLength=0
  var MaxVal = distanceData[0]
  for (let i = 0; i < timeData.length; i++) {
    let DataDate = new Date(timeData[i] * 1000)
    //don't add point if away from desk
    if (awayDesk[i] == false && distanceData[i] < 2000){
      //Monday
      if(DataDate.getDay() == 1){
        MonDistanceSum += distanceData[i]
        MonDataLength++
      }
      //Tuesday
      if(DataDate.getDay() == 2){
        TueDistanceSum += distanceData[i]
        TueDataLength++
      }
      //Wednesday
      if(DataDate.getDay() == 3){
        WedDistanceSum += distanceData[i]
        WedDataLength++
      }
      //Thursday
      if(DataDate.getDay() == 4){
        ThuDistanceSum += distanceData[i]
        ThuDataLength++
      }
      //Friday
      if(DataDate.getDay() == 5){
        FriDistanceSum += distanceData[i]
        FriDataLength++
      }
      //Saturday
      if(DataDate.getDay() == 6){
        SatDistanceSum += distanceData[i]
        SatDataLength++
      }
      //Sunday
      if(DataDate.getDay() == 0){
        SunDistanceSum += distanceData[i]
        SunDataLength++
      }
      //To push up graph
      if(distanceData[i] > MaxVal && distanceData[i] < 2000){
        MaxVal = distanceData[i] 
      }
    }
  }

  BarData[0] = (MonDistanceSum / MonDataLength).toFixed(1)
  BarData[1] = (TueDistanceSum / TueDataLength).toFixed(1)
  BarData[2] = (WedDistanceSum / WedDataLength).toFixed(1)
  BarData[3] = (ThuDistanceSum / ThuDataLength).toFixed(1)
  BarData[4] = (FriDistanceSum / FriDataLength).toFixed(1)
  BarData[5] = (SatDistanceSum / SatDataLength).toFixed(1)
  BarData[6] = (SunDistanceSum / SunDataLength).toFixed(1)

  //set bar colours
  var colours = []
  for (let i = 0; i < BarData.length; i++) {
    var colour = '#51cda0'
    if(BarData[i] > UpperLine || BarData[i] < LowerLine){
      colour = '#e6214f'
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
            label: 'Average Posture',
            backgroundColor: colours,
            borderColor: 'rgba(0, 0, 0, 0)',
            borderWidth: 1,
            data: BarData
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
                zeroLineWidth:3,
                zeroLineColor:'#98999e'
              },
              ticks:{
                fontSize:14,
                fontColor:'#98999e',
                stepSize:1,
                beginAtZero:true,
                stepSize: 250,
                suggestedMin: 250,
                suggestedMax: 1250
              }
            }],
            xAxes:[{
              gridLines:{
                zeroLineWidth:3,
                zeroLineColor:'#98999e'
              },
              ticks:{
                fontSize:14,
                fontColor:'#98999e'
              }
            }]
          },
          annotation:{
            annotations:[
              {
              borderColor:'black',
              borderWidth:2,
              borderDash:[10],
              mode:'horizontal',
              type:'line',
              value:MidPoint,
              scaleID:'y-axis-0'
              }, {
              borderColor:'#6d78ad',
              borderWidth:2,
              borderDash:[10],
              mode:'horizontal',
              type:'line',
              value:UpperLine,
              scaleID:'y-axis-0',
              label: {
                  backgroundColor: "rgba(0, 0, 0, 0)",
                  fontColor: '#98999e',
                  position: 'left',
                  content: "Too Far",
                  enabled: true,
                  yAdjust: -10

              }}, {
              borderColor:'#6d78ad',
              borderWidth:2,
              borderDash:[10],
              mode:'horizontal',
              type:'line',
              value:LowerLine,
              scaleID:'y-axis-0',
              label: {
                  backgroundColor: "rgba(0, 0, 0, 0)",
                  fontColor: '#98999e',
                  position: 'left',
                  content: "Too Close",
                  enabled: true,
                  yAdjust: 12
              }}
            ]
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


export default WeeklyPostureBarChart