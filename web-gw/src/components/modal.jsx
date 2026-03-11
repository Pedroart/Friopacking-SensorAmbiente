import { useEffect } from 'preact/hooks';
import { createPortal } from 'preact/compat';
import './modal.css';

export default function Modal({ isOpen, onClose, onConfirm, title, message, confirmText = 'Confirmar', cancelText = 'Cancelar', type = 'info' }) {
    if (!isOpen) return null;

    useEffect(() => {
        const handleEsc = (e) => {
            if (e.key === 'Escape') onClose();
        };
        window.addEventListener('keydown', handleEsc);
        return () => window.removeEventListener('keydown', handleEsc);
    }, [onClose]);

    const modalComponent = (
        <div className="modal-overlay" onClick={onClose}>
            <div className="modal-content" onClick={e => e.stopPropagation()}>
                <div className="modal-header">
                    <h3>{title}</h3>
                    <p>{message}</p>
                </div>
                <div className="modal-footer">
                    <button className="modal-btn cancel" onClick={onClose}>
                        {cancelText}
                    </button>
                    <button 
                        className={`modal-btn ${type === 'danger' ? 'danger' : 'confirm'}`} 
                        onClick={() => {
                            onConfirm();
                            onClose();
                        }}
                    >
                        {confirmText}
                    </button>
                </div>
            </div>
        </div>
    );

    return createPortal(modalComponent, document.body);
}
