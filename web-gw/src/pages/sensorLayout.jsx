import { useState, useMemo } from 'preact/hooks';
import './sensorLayout.css';

// Mock data para previsualizar, con nombres y datos realistas para los sensores
const mockActiveSensors = Array.from({ length: 15 }, (_, i) => ({ 
    id: `A-${String(i+1).padStart(2, '0')}`, 
    name: `Sensor Ambiente ${i+1}`, 
    temp: (Math.random() * 5 + 20).toFixed(1),
    type: 'active'
}));

const mockAvailableSensors = Array.from({ length: 25 }, (_, i) => ({ 
    id: `D-${String(i+1).padStart(2, '0')}`, 
    name: `Módulo Friopacking ${i+1}`, 
    mac: `00:1A:2B:3C:4D:${(i+10).toString(16).toUpperCase()}` ,
    type: 'available'
}));

export default function SensorLayout() {
    const [searchActive, setSearchActive] = useState('');
    const [searchAvailable, setSearchAvailable] = useState('');
    const [showSearchActive, setShowSearchActive] = useState(false);
    const [showSearchAvailable, setShowSearchAvailable] = useState(false);

    const [selectedSlot, setSelectedSlot] = useState(null);
    const [isSheetOpen, setIsSheetOpen] = useState(false);
    const [sheetSearch, setSheetSearch] = useState('');

    // total number of physical slots per tunnel (max 40)
    const [totalSlots, setTotalSlots] = useState(16);
    const addSlots = () => setTotalSlots(prev => Math.min(40, prev + 4));
    const removeSlots = () => {
        // No eliminar slots ocupados
        const slotsToRemove = Array.from({length: 4}, (_, i) => `slot-${totalSlots - i}`);
        const occupied = slotsToRemove.some(slotId => assignedSlots[slotId]);
        if (occupied) {
            alert('No se pueden eliminar espacios que están ocupados. Desasigne primero los sensores.');
            return;
        }
        setTotalSlots(prev => Math.max(16, prev - 4));
    };

    // helper to build slot array
    const generateSlots = (count) => Array.from({ length: count }, (_, i) => ({ id: `slot-${i+1}`, label: `${i+1}` }));

    // initially assign active sensors into slots up to the current slot count
    const [assignedSlots, setAssignedSlots] = useState(() => {
        const init = {};
        const count = Math.min(mockActiveSensors.length, totalSlots);
        mockActiveSensors.slice(0, count).forEach((s, i) => {
            init[`slot-${i+1}`] = s;
        });
        return init;
    });

    // drag/move support - payload may come from slot or list
    const handleDragStart = (e, payload) => {
        if (!payload) {
            e.preventDefault();
            return;
        }
        e.currentTarget.classList.add('dragging');
        const data = JSON.stringify(payload);
        try {
            e.dataTransfer.setData('application/json', data);
        } catch (err) {
            // Fallback for older browsers or restricted environments
        }
        e.dataTransfer.setData('text/plain', data);
        e.dataTransfer.effectAllowed = 'all';
    };


    const handleDragOver = e => {
        e.preventDefault();
        e.dataTransfer.dropEffect = 'copy';
    };


    const handleDrop = (e, targetSlot) => {
        e.preventDefault();
        e.currentTarget.classList.remove('drag-over');
        let jsonData = e.dataTransfer.getData('application/json');
        if (!jsonData) {
            // try fallback type
            jsonData = e.dataTransfer.getData('text/plain');
        }
        if (!jsonData) return;
        let payload;
        try { payload = JSON.parse(jsonData); } catch { return; }

        // payload may come from sidebar or another slot
        if (payload.source === 'list' && payload.sensor) {
            const sensor = payload.sensor;
            setAssignedSlots(prev => {
                const newSlots = { ...prev };
                Object.keys(newSlots).forEach(k => {
                    if (newSlots[k].id === sensor.id) delete newSlots[k];
                });
                newSlots[targetSlot.id] = sensor;
                return newSlots;
            });
        } else if (payload.source === 'slot' && payload.slotId) {
            const slotId = payload.slotId;
            if (slotId === targetSlot.id) return;
            setAssignedSlots(prev => {
                const newSlots = { ...prev };
                const moving = newSlots[slotId];
                if (!moving) return prev;
                if (newSlots[targetSlot.id]) {
                    newSlots[slotId] = newSlots[targetSlot.id];
                } else {
                    delete newSlots[slotId];
                }
                newSlots[targetSlot.id] = moving;
                return newSlots;
            });
        } else if (payload.id) {
            // fallback: treat payload as sensor object
            const sensor = payload;
            setAssignedSlots(prev => {
                const newSlots = { ...prev };
                Object.keys(newSlots).forEach(k => {
                    if (newSlots[k].id === sensor.id) delete newSlots[k];
                });
                newSlots[targetSlot.id] = sensor;
                return newSlots;
            });
        }
    };

    const handleDragEnter = e => {
        e.currentTarget.classList.add('drag-over');
    };
    const handleDragLeave = e => {
        e.currentTarget.classList.remove('drag-over');
    };
    const handleDragEnd = e => {
        e.currentTarget.classList.remove('dragging');
    };

    const slots = useMemo(() => generateSlots(totalSlots), [totalSlots]);

    const unassignedSensors = useMemo(() => {
        const assignedIds = new Set(Object.values(assignedSlots).map(s => s.id));
        const list = [];
        mockActiveSensors.forEach(s => { if (!assignedIds.has(s.id)) list.push({...s, type: 'active'}); });
        mockAvailableSensors.forEach(s => { if (!assignedIds.has(s.id)) list.push({...s, type: 'available'}); });
        return list;
    }, [assignedSlots]);

    const filteredUnassigned = unassignedSensors.filter(s => 
        s.name.toLowerCase().includes(sheetSearch.toLowerCase()) || 
        s.id.toLowerCase().includes(sheetSearch.toLowerCase()) ||
        (s.mac && s.mac.toLowerCase().includes(sheetSearch.toLowerCase()))
    );

    const handleSlotClick = (slot) => {
        const assigned = assignedSlots[slot.id];
        if (assigned) {
            const confirmRemove = window.confirm(`¿Desea desasignar "${assigned.name}" de este espacio?`);
            if (confirmRemove) {
                const newAssigned = { ...assignedSlots };
                delete newAssigned[slot.id];
                setAssignedSlots(newAssigned);
            }
        } else {
            setSelectedSlot(slot);
            setIsSheetOpen(true);
            setSheetSearch('');
        }
    };


    const handleAssignSensor = (sensor) => {
        if (!selectedSlot) return;
        setAssignedSlots(prev => ({
            ...prev,
            [selectedSlot.id]: sensor
        }));
        setIsSheetOpen(false);
        setTimeout(() => setSelectedSlot(null), 300);
    };

    const handleCloseSheet = () => {
        setIsSheetOpen(false);
        setTimeout(() => setSelectedSlot(null), 300);
    };

    // only show sensors that are not already assigned to a slot
    const assignedIds = useMemo(() => new Set(Object.values(assignedSlots).map(s => s.id)), [assignedSlots]);

    // Mostrar todos los sensores activos, con badge si están asignados
    const filteredActive = mockActiveSensors
        .filter(s =>
            s.name.toLowerCase().includes(searchActive.toLowerCase()) || 
            s.id.toLowerCase().includes(searchActive.toLowerCase())
        );

    const filteredAvailable = mockAvailableSensors
        .filter(s => !assignedIds.has(s.id))
        .filter(s =>
            s.name.toLowerCase().includes(searchAvailable.toLowerCase()) || 
            s.mac.toLowerCase().includes(searchAvailable.toLowerCase())
        );


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
                            <div key={sensor.id} 
                                className={`sensor-item active-sensor ${assignedIds.has(sensor.id) ? 'assigned-in-list' : ''}`} 
                                draggable 
                                onDragStart={e => handleDragStart(e, { source: 'list', sensor })}
                                onDragEnd={handleDragEnd}
                            >

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
                            <div key={sensor.id} 
                                className={`sensor-item active-sensor ${assignedIds.has(sensor.id) ? 'assigned-in-list' : ''}`} 
                                draggable 
                                onDragStart={e => handleDragStart(e, { source: 'list', sensor })}
                                onDragEnd={handleDragEnd}
                            >

                                <div className="sensor-info">
                                    <span className="sensor-name">{sensor.name}</span>
                                    <span className="sensor-id">ID: {sensor.id}</span>
                                </div>
                                <div className="sensor-value">Stock</div>
                            </div>
                        ))}
                        {filteredAvailable.length === 0 && <div className="empty-state">No hay sensores disponibles.</div>}
                    </div>
                </div>
            </div>


            {/* Panel Principal Derecho: Espacios Físicos (Plantillas) */}
            <div className="sensor-main-area">
                <div className="slots-header compact-header">
                    <h2>Asignación a Espacios Físicos ({totalSlots} espacios)</h2>
                    <div className="slots-header-actions">
                        <button className="add-slot-btn" onClick={addSlots} disabled={totalSlots >= 40}>
                            +4
                        </button>
                        <button className="remove-slot-btn" onClick={removeSlots} disabled={totalSlots <= 16}>
                            -4
                        </button>
                        <div className="slot-legend">
                            <span className="legend-item occupied"></span> Ocupado
                            <span className="legend-item empty"></span> Vacío
                        </div>
                    </div>
                </div>

                
                <div className="slots-container custom-scroll">
                    {slots.map(slot => {
                        const assigned = assignedSlots[slot.id];
                        return (
                            <div 
                                key={slot.id} 
                                className={`slot-item ${assigned ? 'assigned' : 'empty-slot'}`}
                                onClick={() => handleSlotClick(slot)}
                                title={assigned ? 'Clic para desasignar / arrastra para mover' : 'Clic o arrastra para asignar sensor'}
                                draggable={!!assigned}
                                onDragStart={e => { e.currentTarget.classList.add('dragging'); handleDragStart(e, { source: 'slot', slotId: slot.id, sensor: assigned }); }}
                                onDragEnd={handleDragEnd}
                                onDragEnter={handleDragEnter}
                                onDragLeave={handleDragLeave}
                                onDragOver={handleDragOver}
                                onDrop={e => handleDrop(e, slot)}
                            >
                                <div className="slot-number">{slot.label}</div>
                                {assigned ? (
                                    <>
                                        <button className="unassign-btn" onClick={(e) => { e.stopPropagation(); handleSlotClick(slot); }} title="Desasignar sensor">
                                            <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="3"><line x1="18" y1="6" x2="6" y2="18"></line><line x1="6" y1="6" x2="18" y2="18"></line></svg>
                                        </button>
                                        <div className="slot-assigned-info">
                                            <span className="assigned-name" title={assigned.name}>{assigned.name}</span>
                                            {assigned.temp ? (
                                                <span className="assigned-val temp">{assigned.temp}°C</span>
                                            ) : (
                                                <span className="assigned-val mac">{assigned.mac ? assigned.mac.substring(12) : 'Stock'}</span>
                                            )}
                                        </div>
                                    </>
                                ) : (
                                    <div className="slot-placeholder">Vacío</div>
                                )}

                            </div>
                        );
                    })}
                </div>
            </div>

            {/* Overlay y Side Sheet para Asignación */}
            {isSheetOpen && (
                <div className="sheet-overlay fade-in" onClick={handleCloseSheet}></div>
            )}
            <div className={`assignment-sheet ${isSheetOpen ? 'open' : ''}`}>
                <div className="sheet-header">
                    <div className="sheet-title">
                        <h3>Asignar Sensor</h3>
                        <p>Espacio Físico {selectedSlot?.label}</p>
                    </div>
                    <button className="icon-action-btn close-sheet-btn" onClick={handleCloseSheet} title="Cerrar panel">
                        <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="18" y1="6" x2="6" y2="18"></line><line x1="6" y1="6" x2="18" y2="18"></line></svg>
                    </button>
                </div>
                <div className="sheet-body custom-scroll">
                    <div className="search-wrapper sheet-search">
                        <span className="search-icon">
                            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="8"></circle><line x1="21" y1="21" x2="16.65" y2="16.65"></line></svg>
                        </span>
                        <input 
                            type="text" 
                            placeholder="Buscar sensor por nombre, ID..." 
                            value={sheetSearch}
                            onInput={e => setSheetSearch(e.target.value)}
                        />
                        {sheetSearch && (
                            <button className="clear-search-btn" onClick={() => setSheetSearch('')}>
                                <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="18" y1="6" x2="6" y2="18"></line><line x1="6" y1="6" x2="18" y2="18"></line></svg>
                            </button>
                        )}
                    </div>

                    <div className="sheet-list">
                        <div className="sheet-section-title">Sensores Disponibles ({filteredUnassigned.length})</div>
                        {filteredUnassigned.length > 0 ? (
                            filteredUnassigned.map(sensor => (
                                <div key={sensor.id} className="sensor-item sheet-sensor-item">
                                    <div className="sensor-info">
                                        <span className="sensor-name">{sensor.name}</span>
                                        <span className="sensor-id">
                                            {sensor.type === 'active' ? `ID: ${sensor.id}` : `MAC: ${sensor.mac}`}
                                        </span>
                                    </div>
                                    <button className="btn-assign" onClick={() => handleAssignSensor(sensor)}>
                                        Asignar
                                    </button>
                                </div>
                            ))
                        ) : (
                            <div className="empty-state">No hay sensores que coincidan con la búsqueda.</div>
                        )}
                    </div>
                </div>
            </div>
        </div>
    );
}
