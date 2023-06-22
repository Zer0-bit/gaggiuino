# Web Interface for Gaggiuino

## Deployment of the WebInterface to ESP

1. Run `npm run build`
2. Use the `webserver` upload action on PIO

## Development
There's a very convenient way to develop the app. 

1. Connect your ESP to the WiFi. 
2. Update `vite.config.ts` `target` to point to the IP of your ESP in your network i.e.
```
  const target = 'http://192.168.4.1';
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

Launches the jest test runner in the interactive watch mode.\
See the section about [running tests](https://jestjs.io/) for more information.

### `npm run build`

Builds the app for production to the `../data` folder.\
It correctly bundles React in production mode and optimizes the build for the best performance.

The build is minified and the filenames include the hashes.\
Your app is ready to be deployed!

See the section about [build](https://vitejs.dev/guide/build.html) for more information.

## Source

https://vitejs.dev