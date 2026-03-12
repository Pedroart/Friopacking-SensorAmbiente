// src/services/networkApi.js
const API_BASE = '/api';
const IS_DEV = import.meta.env.DEV; // Vite detecta si estamos en entorno de desarrollo

// Datos MOCK iniciales para desarrollo (simulando la respuesta en memoria global network del hardware real)
let mockNetworkState = {
  eth: {
    net: { dhcp: true, ip: "192.168.1.100", gateway: "192.168.1.1", subnet: "255.255.255.0", dns1: "8.8.8.8", dns2: "1.1.1.1" }
  },
  ap: {
    ssid: "MiAP", password: "12345678", channel: 6, hidden: false, max_clients: 4,
    net: { dhcp: false, ip: "192.168.4.1", gateway: "192.168.4.1", subnet: "255.255.255.0", dns1: "8.8.8.8", dns2: "1.1.1.1" }
  },
  sta: {
    ssid: "MiWifi", password: "clavewifi",
    net: { dhcp: true, ip: "0.0.0.0", gateway: "0.0.0.0", subnet: "0.0.0.0", dns1: "0.0.0.0", dns2: "0.0.0.0" }
  }
};

/**
 * Función genérica para manejar las peticiones a la API del Gateway
 */
async function fetchApi(endpoint, options = {}) {
    // ------------------------------------------------------------------------
    // INTERCEPTOR PARA ENTORNO DE DESARROLLO (MOCK DATA)
    // Evita el error "Formato de respuesta inválido" cuando el dev server devuelve index.html
    // ------------------------------------------------------------------------
    if (IS_DEV) {
        console.log(`[MOCK API] Llamada a: ${endpoint}`, options);
        await new Promise(r => setTimeout(r, 600)); // Simula retardo de red
        
        if (endpoint === '/network') return mockNetworkState;
        if (endpoint === '/network/status') return { ip: "192.168.1.50" };
        if (endpoint === '/device/info') return { device: "gateway" };
        
        if (endpoint.startsWith('/network/') && options.method === 'POST') {
            const body = JSON.parse(options.body);
            if (endpoint.includes('ethernet')) {
                mockNetworkState.eth = { ...mockNetworkState.eth, ...body };
                return { success: true, message: "Configuración Ethernet actualizada (Mock)" };
            }
            if (endpoint.includes('ap')) {
                mockNetworkState.ap = { ...mockNetworkState.ap, ...body };
                return { success: true, message: "Configuración AP actualizada (Mock)" };
            }
            if (endpoint.includes('sta')) {
                mockNetworkState.sta = { ...mockNetworkState.sta, ...body };
                return { success: true, message: "Configuración STA actualizada (Mock)" };
            }
        }
    }
    // ------------------------------------------------------------------------

    try {
        const res = await fetch(`${API_BASE}${endpoint}`, {
            ...options,
            headers: {
                'Content-Type': 'application/json',
                ...options.headers,
            },
        });

        const textResponse = await res.text();
        let data;
        
        try {
            data = textResponse ? JSON.parse(textResponse) : {};
        } catch (err) {
            console.error('Error parseando JSON:', textResponse);
            throw new Error('Formato de respuesta inválido del servidor');
        }

        if (!res.ok) {
            throw new Error(data.message || `Error del servidor: ${res.status}`);
        }

        if (data && typeof data.success !== 'undefined' && !data.success) {
            throw new Error(data.message || 'Error en la operación');
        }

        return data;
    } catch (error) {
        console.error(`[API Network] Endpoint ${endpoint} falló:`, error);
        throw error;
    }
}

export const networkApi = {
    /**
     * Obtiene la IP actual de conexión
     */
    getStatus: () => fetchApi('/network/status'),

    /**
     * Obtiene información del dispositivo (tipo)
     */
    getDeviceInfo: () => fetchApi('/device/info'),

    /**
     * Obtiene toda la configuración de red guardada actual
     */
    getNetworkConfig: () => fetchApi('/network'),

    /**
     * Actualiza la configuración de Ethernet
     * @param {Object} payload { net: { dhcp, ip, gateway, subnet, dns1, dns2 } }
     */
    updateEthernet: (payload) => fetchApi('/network/ethernet', {
        method: 'POST',
        body: JSON.stringify(payload)
    }),

    /**
     * Actualiza la configuración del Access Point
     * @param {Object} payload { ssid, password, channel, hidden, max_clients, net: {...} }
     */
    updateAP: (payload) => fetchApi('/network/ap', {
        method: 'POST',
        body: JSON.stringify(payload)
    }),

    /**
     * Actualiza la configuración de WiFi Estación
     * @param {Object} payload { ssid, password, net: {...} }
     */
    updateSTA: (payload) => fetchApi('/network/sta', {
        method: 'POST',
        body: JSON.stringify(payload)
    })
};
