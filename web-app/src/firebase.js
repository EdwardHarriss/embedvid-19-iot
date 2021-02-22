import firebase from 'firebase'

var firebaseConfig = {
  apiKey: "AIzaSyBKXJkGTS25VFCG-6eSj4pMBcqdn1kcWgg",
  authDomain: "embedvid-project.firebaseapp.com",
  databaseURL: "https://embedvid-project-default-rtdb.europe-west1.firebasedatabase.app",
  projectId: "embedvid-project",
  storageBucket: "embedvid-project.appspot.com",
  messagingSenderId: "25769248566",
  appId: "1:25769248566:web:6a8b65fee0c2a920dad597",
  measurementId: "G-0LC9XHJ6TF"
};

firebase.initializeApp(firebaseConfig);
export default firebase;