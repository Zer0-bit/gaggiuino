# Web Interface for Gaggiuino

## Deployment of the WebInterface to ESP

1. Run `npm run build`
2. Copy the `/build` directory to `../data`
3.  Use the `Upload Filesystem Image` action on PIO

## Development
There's a very convenient way to develop the app. 

1. Connect your ESP to the WiFi. 
2. Update `src/setupProxy.js` `target` to point to the IP of your ESP in your network i.e.
```
  const target = 'http://192.168.18.44';
```
3. During development also upgrade the websocket path in `Home.jsx` (replace `${window.location.host}` with the IP of ESP32)
```
  const [socketUrl] = useState(`ws://${window.location.host}/ws`);
```
4. Run `npm start`. 
5. Go to `http://localhost:3000`
6. Start changing stuff. Your app will update automatically.


## Available Scripts

In the project directory, you can run:

### `npm start`

Runs the app in the development mode.\
Open [http://localhost:3000](http://localhost:3000) to view it in your browser.

The page will reload when you make changes.\
You may also see any lint errors in the console.

### `npm test`

Launches the test runner in the interactive watch mode.\
See the section about [running tests](https://facebook.github.io/create-react-app/docs/running-tests) for more information.

### `npm run build`

Builds the app for production to the `build` folder.\
It correctly bundles React in production mode and optimizes the build for the best performance.

The build is minified and the filenames include the hashes.\
Your app is ready to be deployed!

See the section about [deployment](https://facebook.github.io/create-react-app/docs/deployment) for more information.

## Source
This project was bootstrapped with [Create React App](https://github.com/facebook/create-react-app).