import { useState } from 'preact/hooks';
import './sensorLayout.css';

// Mock data para previsualizar, con nombres y datos realistas para los sensores
const mockActiveSensors = Array.from({ length: 15 }, (_, i) => ({ 
    id: `A-${String(i+1).padStart(2, '0')}`, 
    name: `Sensor Ambiente ${i+1}`, 
    temp: (Math.random() * 5 + 20).toFixed(1) 
}));

const mockAvailableSensors = Array.from({ length: 25 }, (_, i) => ({ 
    id: `D-${String(i+1).padStart(2, '0')}`, 
    name: `Módulo Friopacking ${i+1}`, 
    mac: `00:1A:2B:3C:4D:${(i+10).toString(16).toUpperCase()}` 
}));

export default function SensorLayout() {
    const [searchActive, setSearchActive] = useState('');
    const [searchAvailable, setSearchAvailable] = useState('');
    const [showSearchActive, setShowSearchActive] = useState(false);
    const [showSearchAvailable, setShowSearchAvailable] = useState(false);

    const filteredActive = mockActiveSensors.filter(s => 
        s.name.toLowerCase().includes(searchActive.toLowerCase()) || 
        s.id.toLowerCase().includes(searchActive.toLowerCase())
    );

    const filteredAvailable = mockAvailableSensors.filter(s => 
        s.name.toLowerCase().includes(searchAvailable.toLowerCase()) || 
        s.mac.toLowerCase().includes(searchAvailable.toLowerCase())
    );

    // Generar un máximo de 40 espacios, segmentados en plantillas de 8 en 8 (5 plantillas)
    const groups = [];
    const BLOCKS_OF = 8;
    const MAX_SLOTS = 40;
    const numberOfGroups = Math.ceil(MAX_SLOTS / BLOCKS_OF);

    for (let i = 0; i < numberOfGroups; i++) {
        const slots = [];
        for (let j = 0; j < BLOCKS_OF; j++) {
            slots.push({ id: `slot-${i * BLOCKS_OF + j + 1}`, label: `${i * BLOCKS_OF + j + 1}` });
        }
        groups.push({ id: `group-${i + 1}`, name: `Plantilla Modelo-${i + 1}`, slots });
    }

    return (
        <div className="sensor-layout-container fade-in">
            <div className="sensor-sidebar">
                {/* Caja Superior: Sensores Activos */}
                <div className="sensor-box">
                    <div className="sensor-box-header inline-header">
                        <h3>Sensores Activos</h3>
                        <div className="header-actions">
                            {showSearchActive || searchActive ? (
                                <div className="search-wrapper compact">
                                    <span className="search-icon">
                                        <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="8"></circle><line x1="21" y1="21" x2="16.65" y2="16.65"></line></svg>
                                    </span>
                                    <input 
                                        type="text" 
                                        placeholder="Buscar..." 
                                        value={searchActive}
                                        onInput={e => setSearchActive(e.target.value)}
                                        autoFocus
                                    />
                                    <button className="clear-search-btn" title="Cerrar búsqueda" onClick={() => { setSearchActive(''); setShowSearchActive(false); }}>
                                        <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="18" y1="6" x2="6" y2="18"></line><line x1="6" y1="6" x2="18" y2="18"></line></svg>
                                    </button>
                                </div>
                            ) : (
                                <button className="icon-action-btn" title="Buscar sensor" onClick={() => setShowSearchActive(true)}>
                                    <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="8"></circle><line x1="21" y1="21" x2="16.65" y2="16.65"></line></svg>
                                </button>
                            )}
                        </div>
                    </div>
                    <div className="sensor-list">
                        {filteredActive.map(sensor => (
                            <div key={sensor.id} className="sensor-item active-sensor">
                                <div className="sensor-info">
                                    <span className="sensor-name">{sensor.name}</span>
                                    <span className="sensor-id">ID: {sensor.id}</span>
                                </div>
                                <div className="sensor-value">{sensor.temp}°C</div>
                            </div>
                        ))}
                        {filteredActive.length === 0 && <div className="empty-state">No se encontraron sensores activos.</div>}
                    </div>
                </div>

                {/* Caja Inferior: Sensores Disponibles */}
                <div className="sensor-box">
                    <div className="sensor-box-header inline-header">
                        <h3>Sensores Disponibles</h3>
                        <div className="header-actions">
                            {showSearchAvailable || searchAvailable ? (
                                <div className="search-wrapper compact">
                                    <span className="search-icon">
                                        <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="8"></circle><line x1="21" y1="21" x2="16.65" y2="16.65"></line></svg>
                                    </span>
                                    <input 
                                        type="text" 
                                        placeholder="Buscar..." 
                                        value={searchAvailable}
                                        onInput={e => setSearchAvailable(e.target.value)}
                                        autoFocus
                                    />
                                    <button className="clear-search-btn" title="Cerrar búsqueda" onClick={() => { setSearchAvailable(''); setShowSearchAvailable(false); }}>
                                        <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="18" y1="6" x2="6" y2="18"></line><line x1="6" y1="6" x2="18" y2="18"></line></svg>
                                    </button>
                                </div>
                            ) : (
                                <button className="icon-action-btn" title="Buscar sensor" onClick={() => setShowSearchAvailable(true)}>
                                    <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="8"></circle><line x1="21" y1="21" x2="16.65" y2="16.65"></line></svg>
                                </button>
                            )}
                        </div>
                    </div>
                    <div className="sensor-list">
                        {filteredAvailable.map(sensor => (
                            <div key={sensor.id} className="sensor-item available-sensor">
                                <div className="sensor-info">
                                    <span className="sensor-name">{sensor.name}</span>
                                    <span className="sensor-mac">MAC: {sensor.mac}</span>
                                </div>
                                {/* Para futuro Drag&Drop o click para asignar */}
                                <button className="add-btn" title="Asignar al espacio">
                                    <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="12" y1="5" x2="12" y2="19"></line><line x1="5" y1="12" x2="19" y2="12"></line></svg>
                                </button>
                            </div>
                        ))}
                        {filteredAvailable.length === 0 && <div className="empty-state">No se encontraron sensores disponibles.</div>}
                    </div>
                </div>
            </div>

            {/* Panel Principal Derecho: Espacios Físicos (Plantillas) */}
            <div className="sensor-main-area">
                <div className="slots-header compact-header">
                    <h2>Asignación a Espacios Físicos</h2>
                </div>
                
                <div className="slots-container custom-scroll">
                    {groups.map((group) => (
                        <div key={group.id} className="slot-group">
                            <div className="group-title-wrapper">
                                <h4 className="group-title">{group.name}</h4>
                                <span className="group-badge">8 Slots</span>
                            </div>
                            <div className="slot-grid">
                                {group.slots.map(slot => (
                                    <div key={slot.id} className="slot-item empty-slot">
                                        <div className="slot-number">{slot.label}</div>
                                        <div className="slot-placeholder">Vacío</div>
                                    </div>
                                ))}
                            </div>
                        </div>
                    ))}
                </div>
            </div>
        </div>
    );
}
