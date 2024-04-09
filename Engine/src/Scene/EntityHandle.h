#pragma once


namespace Engine {
	class EntityHandle{
	public:
		EntityHandle(UUID id) : m_ID(id) {}
		EntityHandle() {
			m_ID = UUID();
		}

		UUID& GetID() { return m_ID; }

		bool operator==(EntityHandle& other) {
			return other.GetID() == m_ID;
		}
	private:
		UUID m_ID;
	};
}