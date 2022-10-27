import React, { useEffect, useState } from 'react';
import PropTypes from 'prop-types';
import { Alert } from '@mui/material';
import { getAvailableNetworks } from '../../client/WifiClient';
import Loader from '../../loader/Loader';
import AvailableNetwork from './AvailableNetwork';

export default function AvailableNetworks({ onConnected }) {
  const [networks, setNetworks] = useState(null);
  const [networksError, setNetworksError] = useState(false);
  const [expandedNetworkId, setExpandedNetworkId] = useState(null);

  useEffect(() => {
    const loadNetworks = async () => {
      if (networks !== null) {
        return;
      }
      try {
        setNetworksError(false);
        const networksResponse = await getAvailableNetworks();
        setNetworks(networksResponse);
      } catch (e) {
        setNetworksError(true);
      }
    };
    loadNetworks();
  }, []);

  if (networksError) {
    return <Alert severity="error">Failed to load available networks</Alert>;
  }

  return networks === null
    ? <Loader />
    : (
      <div className="NetworkList">
        {networks.map((network) => (
          <AvailableNetwork
            key={network.ssid}
            network={network}
            onClick={() => (network.ssid === expandedNetworkId
              ? setExpandedNetworkId(null)
              : setExpandedNetworkId(network.ssid))}
            expanded={expandedNetworkId === network.ssid}
            onConnected={() => {
              setExpandedNetworkId(null);
              onConnected();
            }}
          />
        ))}
      </div>
    );
}

AvailableNetworks.propTypes = {
  onConnected: PropTypes.func,
};

AvailableNetworks.defaultProps = {
  onConnected: () => {},
};
